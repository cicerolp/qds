#include "stdafx.h"
#include "types.h"

#include "NDS.h"

#include "Categorical.h"
#include "Spatial.h"
#include "Temporal.h"

#include "PDigest.h"
#include "Gaussian.h"

NDS::NDS(const Schema &schema) {
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();

  uint32_t pivots_count = 0;

  Data data(schema);

  std::cout << "Input Data: " << std::endl;
  for (auto &file : schema.files) {
    std::cout << "\tFile: " << file << std::endl;
  }
  std::cout << std::endl;

  std::cout << "NDS: " << std::endl;
  std::cout << "\tName: " << schema.name << std::endl;
  std::cout << "\tSize: " << data.size() << std::endl;
  std::cout << std::endl;

  _root = pivot_ctn(1);
  _root[0] = Pivot(0, data.size());

#ifdef NDS_ENABLE_PAYLOAD
  for (auto i = 0; i < schema.payload.size(); ++i) {
    const auto &info = schema.payload[i];

    switch (info.bin) {
      case 0: {
        std::cout << "\tPayload Dimension: PDigest\n\t\t" << info << std::endl;
        _payload.emplace_back(std::make_unique<PDigest>(info));
      }
        break;
      case 1: {
        std::cout << "\tPayload Dimension: Gaussian\n\t\t" << info << std::endl;
        _payload.emplace_back(std::make_unique<Gaussian>(info));
      }
        break;
    }

    // store payload index
    _payload_index[info.index] = i;

    // prepare payload
    data.preparePayload(info.offset);
  }
  std::cout << std::endl;

  // create root payload
  NDS::create_payload(data, _root[0]);
#endif // NDS_ENABLE_PAYLOAD

  BuildPair<link_ctn> links;
  links.input.emplace_back(&_root);

  BuildPair<build_ctn> range;
  range.input.emplace_back(0, data.size());

  for (const auto &info : schema.dimension) {
    switch (info.type) {
      case DimensionSchema::Spatial: {
        std::cout << "\tSpatial Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Spatial>(info));
      }
        break;
      case DimensionSchema::Temporal: {
        std::cout << "\tTemporal Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Temporal>(info));
      }
        break;
      case DimensionSchema::Categorical: {
        std::cout << "\tCategorical Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Categorical>(info));
      }
        break;
      default:std::cerr << "error: invalid NDS" << std::endl;
        std::abort();
        break;
    }

    uint32_t curr_count = _dimension.back()->build(*this, data, range, links);
    pivots_count += curr_count;

    swap_and_clear<link_ctn>(links.input, links.output);
    swap_and_clear<build_ctn>(range.input, range.output);

    std::cout << "\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
  }

  std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

  end = std::chrono::high_resolution_clock::now();
  long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;
}

std::string NDS::query(const Query &query) {
  subset_ctn subsets;

  RangePivot root(_root[0]);

  for (auto &d : _dimension) {
    if (d->query(query, subsets) == false) {
      // empty query
      root.pivot.back(0);
      subsets.clear();
      break;
    }
  }

  return serialize(query, subsets, root);
}

std::string NDS::pipeline(const Pipeline &pipeline) {
  subset_ctn source_ctn, dest_ctn;

  RangePivot root(_root[0]);

  for (auto &d : _dimension) {
    if (!d->query(pipeline.get_source(), source_ctn) || !d->query(pipeline.get_dest(), dest_ctn)) {
      // empty query
      root.pivot.back(0);
      source_ctn.clear();
      dest_ctn.clear();
      break;
    }
  }

  return serialize_pipeline(pipeline, source_ctn, dest_ctn, root);
}

std::string NDS::serialize(const Query &query, subset_ctn &subsets, const RangePivot &root) const {
  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  if (subsets.size() == 0) {
    if (!root.pivot.empty()) {
      auto aggregators = get_aggr_none(query);

      range_ctn range{root};

      summarize_range(aggregators, writer, range);
    } else {
      // empty response
      writer.StartArray();
      writer.EndArray();
    }
  } else {
    auto range = get_range(subsets);

    if (query.group_by()) {
      if (subsets.back().option == CopyValueFromSubset) {
        auto aggregators = get_aggr_subset(query, subsets.back().container.size());
        group_by_subset(aggregators, writer, range, subsets.back().container);

      } else {
        // initialize aggregators
        auto aggregators = get_aggr_range(query);
        group_by_range(aggregators, writer, range, subsets.back().container);
      }

    } else {
      auto aggregators = get_aggr_none(query);
      summarize_subset(aggregators, writer, range, subsets.back().container);
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

std::string NDS::serialize_pipeline(const Pipeline &pipeline,
                                    subset_ctn &source_ctn,
                                    subset_ctn &dest_ctn,
                                    const RangePivot &root) const {
  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  if (root.pivot.empty()) {
    // empty response
    writer.StartArray();
    writer.EndArray();
  }

  if (pipeline.get_source().group_by() != pipeline.get_dest().group_by()) {
    // invalid pipeline
    writer.StartArray();
    writer.EndArray();
  }

  // summarize
  if (!pipeline.get_source().group_by()) {    
    auto aggr_source = get_aggr_none(pipeline.get_source());
    auto range_source = get_range(source_ctn);
    auto subset_source = get_subset(source_ctn);

    if (source_ctn.size() == 0) {
      do_summarize_range(aggr_source, range_source);
    } else {
      do_summarize_subset(aggr_source, range_source, subset_source);
    }

    auto aggr_dest = get_aggr_none(pipeline.get_dest());
    auto range_dest = get_range(dest_ctn);
    auto subset_dest = get_subset(dest_ctn);

    if (dest_ctn.size() == 0) {
      do_summarize_range(aggr_dest, range_dest);
    } else {
      do_summarize_subset(aggr_dest, range_dest, subset_dest);
    }

    auto groups = std::make_pair(GroupBy<AggrNoneCtn>(aggr_source, range_source, subset_source),
                                 GroupBy<AggrNoneCtn>(aggr_dest, range_dest, subset_dest));

    summarize_pipe(groups, writer);
  }

  /*if (source_ctn.size() == 0 && dest_ctn.size() == 0) {
    auto aggr_source = get_aggr_none(pipeline.get_source());
    auto aggr_dest = get_aggr_none(pipeline.get_dest());

    range_ctn range{root};
    bined_ctn subset;

    auto groups = std::make_pair(GroupBy<AggrNoneCtn>(aggr_source, range, subset),
                                 GroupBy<AggrNoneCtn>(aggr_dest, range, subset));

    summarize_range(groups, writer);
  } else if (source_ctn.size() != 0 && dest_ctn.size() != 0) {
    auto range_source = get_range(source_ctn);
    auto range_dest = get_range(dest_ctn);

    if (pipeline.get_source().group_by() && pipeline.get_dest().group_by()) {
      if (option == CopyValueFromSubset) {
        auto aggregators = get_aggr_subset(query, subsets.back().container.size());
        group_by_subset(aggregators, writer, range, subsets.back().container);

      } else {
        // initialize aggregators
        auto aggregators = get_aggr_range(query);
        group_by_range(aggregators, writer, range, subsets.back().container);
      }

    } else {
      auto aggr_source = get_aggr_none(pipeline.get_source());
      auto aggr_dest = get_aggr_none(pipeline.get_dest());

      auto groups = std::make_pair(GroupBy<AggrNoneCtn>(aggr_source, range_source, source_ctn.back().container),
                                   GroupBy<AggrNoneCtn>(aggr_dest, range_dest, dest_ctn.back().container));

      summarize_subset(groups, writer);
    }
  }*/

  // end json
  writer.EndArray();
  return buffer.GetString();
}

range_ctn NDS::get_range(const subset_ctn &subsets) const {
  if (subsets.size() == 0) {
    return {_root[0]};
  } else {
    CopyOption option = DefaultCopy;
    range_ctn range, response{_root[0]};

    for (auto i = 0; i < subsets.size() - 1; ++i) {
      restrict(range, response, subsets[i], option);
    }

    if (option == DefaultCopy) option = subsets.back().option;

    // sort range only when necessary
    swap_and_sort(range, response, option);

    return range;
  }
}

bined_ctn NDS::get_subset(const subset_ctn &subsets) const {
  if (subsets.size() == 0) {
    return bined_ctn();
  } else {
    return subsets.back().container;
  }
}

void NDS::restrict(range_ctn &range, range_ctn &response, const subset_t &subset, CopyOption &option) const {
  if (option == DefaultCopy) option = subset.option;

  // sort range only when necessary
  swap_and_sort(range, response, option);

  pivot_it it_lower, it_upper;
  range_it it_range;

  switch (option) {
    case CopyValueFromRange:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          while (it_lower != it_upper) {
            response.emplace_back((*it_lower++), (*it_range).value);
          }
          ++it_range;
        }
      }
      break;
    case CopyValueFromSubset:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          while (it_lower != it_upper) {
            response.emplace_back((*it_lower++), el->value);
          }
          ++it_range;
        }
      }
      break;
    default:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          response.insert(response.end(), it_lower, it_upper);
          it_lower = it_upper;
          ++it_range;
        }
      }
      break;
  }

  if (option == CopyValueFromSubset) option = CopyValueFromRange;
}

bool NDS::search_iterators(range_it &it_range, const range_ctn &range,
                           pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset) const {
  if (it_lower == subset.end() || it_range == range.end()) return false;

  if ((*it_range).pivot.ends_before(*it_lower)) {
    // binary search to find range iterator
    it_range = std::lower_bound(it_range, range.end(), (*it_lower), RangePivot::upper_bound_comp);

    if (it_range == range.end()) return false;
  }

  if ((*it_range).pivot.begins_after(*it_lower)) {
    if (it_lower == std::prev(subset.end())) return false;

    // binnary search to find lower subset iterator
    it_lower = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);

    if (it_lower == subset.end()) return false;
  }

  it_upper = std::upper_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::upper_bound_comp);

  if (it_lower == it_upper) {
    return false;
  }

  return true;
}

void NDS::compactation(range_ctn &input, range_ctn &output, CopyOption option) const {
  output.emplace_back(input.front());

  if (option == DefaultCopy || option == CopyValueFromSubset) {
    for (size_t i = 1; i < input.size(); ++i) {
      if (Pivot::is_sequence(output.back().pivot, input[i].pivot)) {
        output.back().pivot.append(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  } else { // CopyValueFromRange
    for (size_t i = 1; i < input.size(); ++i) {
      if (RangePivot::is_sequence(output.back(), input[i])) {
        output.back().pivot.append(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  }
}

std::string NDS::schema() const {

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();

  writer.Key("count");
  writer.Int(_root[0].size());

  writer.Key("index_dimensions");
  writer.StartArray();
  for (const auto &ptr : _dimension) {
    writer.StartObject();
    ptr->get_schema(writer);
    writer.EndObject();
  }
  writer.EndArray();

  writer.Key("index_payloads");
  writer.StartArray();
  for (const auto &ptr : _payload) {
    writer.StartObject();
    ptr->get_schema(writer);
    writer.EndObject();
  }
  writer.EndArray();

  writer.EndObject();

  return buffer.GetString();
}

void NDS::group_by_subset(AggrSubsetCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const {

  for (auto el = 0; el < subset.size(); ++el) {
    pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
      for (auto &aggr : aggrs) {
        aggr->merge(el, it_lower, it_upper);
      }
      it_lower = it_upper;
      ++it_range;
    }
  }

  for (auto &aggr : aggrs) {
    writer.StartArray();
    for (auto el = 0; el < subset.size(); ++el) {
      aggr->output(el, subset[el]->value, writer);
    }
    writer.EndArray();
  }
}

void NDS::group_by_range(AggrRangeCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const {
  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      for (auto &aggr : aggrs) {
        aggr->merge((*it_range).value, it_lower, it_upper);
      }
      it_lower = it_upper;
      ++it_range;
    }
  }

  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}

void NDS::summarize_subset(AggrNoneCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const {
  do_summarize_subset(aggrs, range, subset);

  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}
void NDS::summarize_range(AggrNoneCtn &aggrs, json &writer, range_ctn &range) const {
  do_summarize_range(aggrs, range);

  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}

void NDS::summarize_pipe(GroupCtn<AggrNoneCtn> &groups, json &writer) const {
  for (auto &source_aggr : groups.first.aggrs) {
    auto pipe = source_aggr->source();

    writer.StartArray();
    for (auto &dest_aggr : groups.second.aggrs) {
      writer.StartArray();
      dest_aggr->output(pipe, writer);
      writer.EndArray();
    }
    writer.EndArray();
  }
}

void NDS::do_summarize_range(AggrNoneCtn &aggrs, range_ctn &range) const {
  for (auto &aggr : aggrs) {
    aggr->merge(range.begin(), range.end());
  }
}

void NDS::do_summarize_subset(AggrNoneCtn &aggrs, range_ctn &range, const bined_ctn &subset) const {
  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      for (auto &aggr : aggrs) {
        aggr->merge(it_lower, it_upper);
      }
      it_lower = it_upper;
      ++it_range;
    }
  }
}

AggrNoneCtn NDS::get_aggr_none(const Query &query) const {
  std::vector<std::shared_ptr<AggrNone>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountNone>(expr, get_payload_index(expr.second)));
    }

#ifdef ENABLE_PDIGEST
    if (expr.first == "quantile" || expr.first == "inverse") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestNone>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianNone>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}

AggrRangeCtn NDS::get_aggr_range(const Query &query) const {
  std::vector<std::shared_ptr<AggrRange>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountRange>(expr, get_payload_index(expr.second)));
    }

#ifdef ENABLE_PDIGEST
    if (expr.first == "quantile" || expr.first == "inverse") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestRange>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianRange>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}

AggrSubsetCtn NDS::get_aggr_subset(const Query &query, size_t subset_size) const {
  std::vector<std::shared_ptr<AggrSubset>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountSubset>(expr, get_payload_index(expr.second), subset_size));
    }

#ifdef ENABLE_PDIGEST
    if (expr.first == "quantile" || expr.first == "inverse") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestSubset>(expr, get_payload_index(expr.second), subset_size));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianSubset>(expr, get_payload_index(expr.second), subset_size));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}
