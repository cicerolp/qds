#include "Dimension.h"

Dimension::Dimension(const std::tuple<uint32_t, uint32_t, uint32_t> &tuple)
    : _key(std::get<0>(tuple)),
      _bin(std::get<1>(tuple)),
      _offset(std::get<2>(tuple)) {
  std::cout << "\t\tKey: [" << _key << "], Bin: [" << _bin << "], Offset: ["
            << _offset << "]" << std::endl;
}

void Dimension::restrict(range_ctn &range, range_ctn &response,
                         const subset_t &subset, CopyOption &option) {
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

std::string Dimension::serialize(const Query &query, subset_ctn &subsets, const RangePivot &root) {
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
      if (aggr.first == "count") {
        group_by_none<AggrCountNone>(query, writer, response);

      } else if (aggr.first == "quantile") {
#ifdef ENABLE_PDIGEST
        group_by_none<AggrQuantileNone>(query, writer, response);
#endif
      } else if (aggr.first == "inverse") {
#ifdef ENABLE_PDIGEST
        group_by_none<AggrInverseNone>(query, writer, response);
#endif
      }
    }
  } else {
    for (auto i = 0; i < subsets.size() - 1; ++i)
      restrict(range, response, subsets[i], option);

    if (option == DefaultCopy) option = subsets.back().option;

    // sort range only when necessary
    swap_and_sort(range, response, option);

    if (aggr.first == "count") {
      if (query.group_by()) {
        if (option == CopyValueFromSubset) {
          // group_by_subset
          group_by_subset<AggrCountSubset>(query, writer, range, subsets.back().container);
        } else {
          // group_by_range
          group_by_range<AggrCountRange>(query, writer, range, subsets.back().container);
        }
      } else {
        // group_by_none
        group_by_none<AggrCountNone>(query, writer, range, subsets.back().container);
      }

    } else if (aggr.first == "quantile") {
#ifdef ENABLE_PDIGEST
      if (query.group_by()) {
        if (option == CopyValueFromSubset) {
          // group_by_subset
          group_by_subset<AggrQuantileSubset>(query, writer, range, subsets.back().container);
        } else {
          // group_by_range
          group_by_range<AggrQuantileRange>(query, writer, range, subsets.back().container);
        }
      } else {
        // group_by_none
        group_by_none<AggrQuantileNone>(query, writer, range, subsets.back().container);
      }
#endif
    } else if (aggr.first == "inverse") {
#ifdef ENABLE_PDIGEST
      if (query.group_by()) {
        if (option == CopyValueFromSubset) {
          // group_by_subset
          group_by_subset<AggrInverseSubset>(query, writer, range, subsets.back().container);
        } else {
          // group_by_range
          group_by_range<AggrInverseRange>(query, writer, range, subsets.back().container);
        }
      } else {
        // group_by_none
        group_by_none<AggrInverseNone>(query, writer, range, subsets.back().container);
      }
#endif
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();

}
