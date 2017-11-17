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
  if (subsets.size() == 0) return std::string("[]");

  CopyOption option = DefaultCopy;
  range_ctn range, response;

  response.emplace_back(root);

  for (auto i = 0; i < subsets.size() - 1; ++i)
    restrict(range, response, subsets[i], option);

  if (option == DefaultCopy) option = subsets.back().option;

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  // sort range only when necessary
  swap_and_sort(range, response, option);

  if (option == CopyValueFromSubset) {
    switch (query.output()) {
      case Query::QueryOutput::COUNT: {
        if (query.aggregation() == Query::QueryAggregation::NONE) {
          write_none(query, writer, range, subsets.back().container);
        } else {
          write_subset<CountSubsetAggr>(query, writer, range, subsets.back().container);
        }
      }
        break;
      case Query::QueryOutput::QUANTILE: {
        if (query.aggregation() == Query::QueryAggregation::NONE) {
          write_none(query, writer, range, subsets.back().container);
        } else {
          write_subset<QuantileSubsetAggr>(query, writer, range, subsets.back().container);
        }
      }
        break;
    }
  } else { //CopyValueFromRange
    switch (query.output()) {
      case Query::QueryOutput::COUNT: {
        if (query.aggregation() == Query::QueryAggregation::NONE) {
          write_none(query, writer, range, subsets.back().container);
        } else {
          write_range<CountRangeAggr>(query, writer, range, subsets.back().container);
        }
      }
        break;
      case Query::QueryOutput::QUANTILE: {
        if (query.aggregation() == Query::QueryAggregation::NONE) {
          write_none(query, writer, range, subsets.back().container);
        } else {
          write_range<QuantileRangeAggr>(query, writer, range, subsets.back().container);
        }
      }
        break;
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

void Dimension::write_none(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                           range_ctn &range, const subset_pivot_ctn &subset) {

  Pivot pdigest;
  uint32_t count = 0;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      if (query.output() == Query::QueryOutput::COUNT) {
        count += aggregate_count(it_lower, it_upper);
      } else {
        pdigest.merge_pdigest(it_lower, it_upper);
      }
      ++it_range;
    }
  }

  if (query.output() == Query::QueryOutput::COUNT) {
    writer.Uint(count);
  } else {
    for (auto &q : query.quantiles()) {
      writer.StartArray();
      writer.Double(q);
      writer.Double(pdigest.quantile(q));
      writer.EndArray();
    }
  }
}
