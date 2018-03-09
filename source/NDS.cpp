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

#ifdef NDS_ENABLE_PAYLOAD
  for (auto &elt : _payload) {
    elt->dispose_buffers();
  }
#endif // NDS_ENABLE_PAYLOAD

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

  if (root.pivot.empty()) {
    // empty response
    writer.StartArray();
    writer.EndArray();

  } else {
    CopyOption option = DefaultCopy;

    auto range = get_range(subsets, option);
    auto subset = get_subset(subsets);

    // summarize
    if (!query.group_by()) {
      auto aggr = get_aggr_summarize(query);
      do_summarize(aggr, range, subset);

      summarize_query(aggr, writer, range, subset);

    } else {
      auto aggr = get_aggr_group_by(query);
      do_group_by(aggr, range, subset, option);

      group_by_query(aggr, writer, range, subset);
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

  } else if (pipeline.get_source().get_group_by() != pipeline.get_dest().get_group_by()) {
    // invalid pipeline
    writer.StartArray();
    writer.EndArray();

  } else {
    CopyOption option_source = DefaultCopy;
    auto range_source = get_range(source_ctn, option_source);
    auto subset_source = get_subset(source_ctn);

    CopyOption option_dest = DefaultCopy;
    auto range_dest = get_range(dest_ctn, option_dest);
    auto subset_dest = get_subset(dest_ctn);

    // summarize
    if (!pipeline.get_source().group_by()) {
      auto aggr_source = get_aggr_summarize(pipeline.get_source());
      do_summarize(aggr_source, range_source, subset_source);

      auto aggr_dest = get_aggr_summarize(pipeline.get_dest());
      do_summarize(aggr_dest, range_dest, subset_dest);

      auto groups = std::make_pair(GroupBy<AggrSummarizeCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrSummarizeCtn>(aggr_dest, range_dest, subset_dest));

      summarize_pipe(groups, writer);

    } else {
      auto aggr_source = get_aggr_group_by(pipeline.get_source());
      do_group_by(aggr_source, range_source, subset_source, option_source);

      auto aggr_dest = get_aggr_group_by(pipeline.get_dest());
      do_group_by(aggr_dest, range_dest, subset_dest, option_dest);

      auto groups = std::make_pair(GroupBy<AggrGroupByCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrGroupByCtn>(aggr_dest, range_dest, subset_dest));

      if (pipeline.get_join() == "inner_join") {
        group_by_inner_join(groups, writer, pipeline.get_threshold());
      } else if (pipeline.get_join() == "left_join") {
        group_by_left_join(groups, writer, pipeline.get_threshold());
      } else if (pipeline.get_join() == "right_join") {
        group_by_right_join(groups, writer, pipeline.get_threshold());
      }
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

range_ctn NDS::get_range(const subset_ctn &subsets, CopyOption &option) const {
  if (subsets.size() == 0) {
    return {_root[0]};
  } else {
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

void NDS::group_by_query(AggrGroupByCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const {
  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}
void NDS::group_by_inner_join(GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    // get keys from source
    for (auto &key : source_aggr->get_mapped_values(threshold)) {
      // get pipe from source
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_two_way(key, pipe, writer, threshold);
      }
    }
    writer.EndArray();
  }
}

void NDS::group_by_left_join(GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    // get keys from source
    for (auto &key : source_aggr->get_mapped_values(0)) {
      // get pipe from sorce
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_one_way(key, pipe, writer);
      }
    }
    writer.EndArray();
  }
}

void NDS::group_by_right_join(GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    // get keys from destination
    for (auto &dest_aggr : groups.second.aggrs) {
      for (auto &key : dest_aggr->get_mapped_values(threshold)) {
        // get pipe from source
        auto pipe = source_aggr->get_pipe(key, 0);
        dest_aggr->output_one_way(key, pipe, writer);
      }
    }
    writer.EndArray();
  }
}

void NDS::do_group_by(AggrGroupByCtn &aggrs,
                      range_ctn &range,
                      const bined_ctn &subset,
                      const CopyOption &option) const {
  if (option == CopyValueFromSubset) {
    for (auto el = 0; el < subset.size(); ++el) {
      pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge(subset[el]->value, it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  } else {
    for (const auto &el : subset) {
      pivot_it it_lower = el->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge((*it_range).value, it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  }
}

void NDS::summarize_query(AggrSummarizeCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const {
  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}

void NDS::summarize_pipe(GroupCtn<AggrSummarizeCtn> &groups, json &writer) const {
  for (auto &source_aggr : groups.first.aggrs) {
    auto pipe = source_aggr->get_pipe();

    writer.StartArray();
    for (auto &dest_aggr : groups.second.aggrs) {
      writer.StartArray();
      dest_aggr->output(pipe, writer);
      writer.EndArray();
    }
    writer.EndArray();
  }
}

void NDS::do_summarize(AggrSummarizeCtn &aggrs, range_ctn &range, const bined_ctn &subset) const {
  if (subset.size() != 0) {
    for (const auto &el : subset) {
      pivot_it it_lower = el->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge(it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  } else {
    for (auto &aggr : aggrs) {
      aggr->merge(range.begin(), range.end());
    }
  }
}

AggrGroupByCtn NDS::get_aggr_group_by(const Query &query) const {
  std::vector<std::shared_ptr<AggrGroupBy>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountGroupBy>(expr, get_payload_index(expr.second)));
    }

#ifdef ENABLE_PDIGEST
    if (expr.first == "quantile" || expr.first == "inverse") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestGroupBy>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianGroupBy>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}

AggrSummarizeCtn NDS::get_aggr_summarize(const Query &query) const {
  std::vector<std::shared_ptr<AggrSummarize>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountSummarize>(expr, get_payload_index(expr.second)));
    }

#ifdef ENABLE_PDIGEST
    if (expr.first == "quantile" || expr.first == "inverse") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestSummarize>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianSummarize>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}
