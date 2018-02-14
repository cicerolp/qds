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

  Data data(schema.file);

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

std::string NDS::serialize(const Query &query, subset_ctn &subsets, const RangePivot &root) const {
  // get aggregation clausule
  auto &aggr = query.get_aggr();

  CopyOption option = DefaultCopy;
  range_ctn range, response;

  response.emplace_back(root);

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  if (subsets.size() == 0) {
    if (!root.pivot.empty()) {
      // initialize aggregators
      std::vector<std::unique_ptr<AggrNone>> aggregators;

      for (const auto &expr : aggr) {
        if (expr.first == "count") {
          aggregators.emplace_back(std::make_unique<AggrCountNone>(expr, get_payload_index(expr.second)));
        }

#ifdef ENABLE_PDIGEST
        if (expr.first == "quantile" || expr.first == "inverse") {
          aggregators.emplace_back(std::make_unique<AggrPDigestNone>(expr, get_payload_index(expr.second)));
        }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
        if (expr.first == "variance" || expr.first == "average") {
          aggregators.emplace_back(std::make_unique<AggrGaussianNone>(expr, get_payload_index(expr.second)));
        }
#endif // ENABLE_GAUSSIAN
      }

      group_by_none(aggregators, writer, response);
    }

  } else {
    for (auto i = 0; i < subsets.size() - 1; ++i) {
      restrict(range, response, subsets[i], option);
    }

    if (option == DefaultCopy) option = subsets.back().option;

    // sort range only when necessary
    swap_and_sort(range, response, option);

    if (query.group_by()) {
      if (option == CopyValueFromSubset) {
        // initialize aggregators
        std::vector<std::unique_ptr<AggrSubset>> aggregators;

        for (const auto &expr : aggr) {
          if (expr.first == "count") {
            aggregators.emplace_back(std::make_unique<AggrCountSubset>(expr,
                                                                       get_payload_index(expr.second),
                                                                       subsets.back().container.size()));
          }

#ifdef ENABLE_PDIGEST
          if (expr.first == "quantile" || expr.first == "inverse") {
            aggregators.emplace_back(std::make_unique<AggrPDigestSubset>(expr,
                                                                         get_payload_index(expr.second),
                                                                         subsets.back().container.size()));
          }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
          if (expr.first == "variance" || expr.first == "average") {
            aggregators.emplace_back(std::make_unique<AggrGaussianSubset>(expr,
                                                                          get_payload_index(expr.second),
                                                                          subsets.back().container.size()));
          }
#endif // ENABLE_GAUSSIAN
        }
        group_by_subset(aggregators, writer, range, subsets.back().container);

        // Range
      } else {
        // initialize aggregators
        std::vector<std::unique_ptr<AggrRange>> aggregators;

        for (const auto &expr : aggr) {
          if (expr.first == "count") {
            aggregators.emplace_back(std::make_unique<AggrCountRange>(expr, get_payload_index(expr.second)));
          }

#ifdef ENABLE_PDIGEST
          if (expr.first == "quantile" || expr.first == "inverse") {
            aggregators.emplace_back(std::make_unique<AggrPDigestRange>(expr, get_payload_index(expr.second)));
          }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
          if (expr.first == "variance" || expr.first == "average") {
            aggregators.emplace_back(std::make_unique<AggrGaussianRange>(expr, get_payload_index(expr.second)));
          }
#endif // ENABLE_GAUSSIAN
        }
        group_by_range(aggregators, writer, range, subsets.back().container);

      }
      // None
    } else {
      // initialize aggregators
      std::vector<std::unique_ptr<AggrNone>> aggregators;

      for (const auto &expr : aggr) {
        if (expr.first == "count") {
          aggregators.emplace_back(std::make_unique<AggrCountNone>(expr, get_payload_index(expr.second)));
        }

#ifdef ENABLE_PDIGEST
        if (expr.first == "quantile" || expr.first == "inverse") {
          aggregators.emplace_back(std::make_unique<AggrPDigestNone>(expr, get_payload_index(expr.second)));
        }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
        if (expr.first == "variance" || expr.first == "average") {
          aggregators.emplace_back(std::make_unique<AggrGaussianNone>(expr, get_payload_index(expr.second)));
        }
#endif // ENABLE_GAUSSIAN
      }
      group_by_none(aggregators, writer, range, subsets.back().container);
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
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
