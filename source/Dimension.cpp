#include "Dimension.h"

Dimension::Dimension(const std::tuple<uint32_t, uint32_t, uint32_t> &tuple)
    : _key(std::get<0>(tuple)),
      _bin(std::get<1>(tuple)),
      _offset(std::get<2>(tuple)) {
  std::cout << "\t\tKey: [" << _key << "], Bin: [" << _bin << "], Offset: ["
            << _offset << "]" << std::endl;
}

void Dimension::restrict(range_container &range, range_container &response,
                         const subset_t &subset, CopyOption &option) {
  if (option == DefaultCopy) option = subset.option;

  // sort range only when necessary
  swap_and_sort(range, response, option);

  pivot_it it_lower, it_upper;
  range_iterator it_range;

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

std::string Dimension::serialize(const Query &query, subset_container &subsets,
                                 const BinnedPivot &root) {
  if (subsets.size() == 0) return std::string("[]");

  CopyOption option = DefaultCopy;
  range_container range, response;

  response.emplace_back(root);

  for (auto i = 0; i < subsets.size() - 1; ++i)
    restrict(range, response, subsets[i], option);

  if (option == DefaultCopy) option = subsets.back().option;

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  switch (query.aggregation()) {
    case Query::TILE: {
      // sort range only when necessary
      swap_and_sort(range, response, option);

      if (option == CopyValueFromSubset) {
        write_subset<spatial_t>(writer, range, subsets.back().container);
      } else {
        write_range<spatial_t, std::unordered_map>(writer, range, subsets.back().container);
      }
    }
      break;
    case Query::GROUP: {
      // sort range only when necessary
      swap_and_sort(range, response, option);

      if (option == CopyValueFromSubset) {
        write_subset<categorical_t>(writer, range, subsets.back().container);
      } else {
        write_range<uint64_t, std::map>(writer, range, subsets.back().container);
      }
    }
      break;
    case Query::TSERIES: {
      // sort range only when necessary
      swap_and_sort(range, response, option);

      if (option == CopyValueFromSubset) {
        write_subset<temporal_t>(writer, range, subsets.back().container);
      } else {
        write_range<uint64_t, std::map>(writer, range, subsets.back().container);
      }
    }
      break;
    case Query::REGION: {
      // sort range only when necessary
      swap_and_sort(range, response, option);

      write_count(writer, range, subsets.back().container);
    }
      break;
    /*case Query::QUANTILE : {
      // sort range only when necessary
      swap_and_sort(range, response, option);

      write_quantile(query, writer, range, subsets.back().container);
    }*/
    default: break;
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

void Dimension::write_count(rapidjson::Writer<rapidjson::StringBuffer> &writer,
                            range_container &range, const binned_ctn &subset) {
  uint32_t count = 0;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_iterator it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      count += count_and_increment(it_lower, it_upper);
      ++it_range;
    }
  }

  writer.StartArray();
  writer.Uint(count);
  writer.EndArray();
}

void Dimension::write_quantile(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                               range_container &range, const binned_ctn &subset) {
  Pivot pdigest;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_iterator it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      while (it_lower != it_upper) {
        pdigest.merge_pdigest((*it_lower++));
      }
      ++it_range;
    }
  }

  writer.StartArray();
  for (auto& q : query.quantiles()) {
    writer.Double(pdigest.quantile(q));
  }
  writer.EndArray();
}

void Dimension::write_pivtos(rapidjson::Writer<rapidjson::StringBuffer> &writer,
                             range_container &range, const binned_ctn &subset) {
  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_iterator it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      while (it_lower != it_upper) {
        writer.StartArray();
        writer.Uint((*it_lower).front());
        writer.Uint((*it_lower++).back());
        writer.EndArray();
      }
      ++it_range;
    }
  }
}


