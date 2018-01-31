#pragma once

#include "RangePivot.h"
#include "Pivot.h"
#include "Query.h"
#include "Aggr.h"

class NDS;
class Data;

class Dimension {
 public:
  Dimension(const DimensionSchema &schema);
  virtual ~Dimension() = default;

  virtual bool query(const Query &query, subset_ctn &subsets) const = 0;

  virtual uint32_t build(BuildPair<build_ctn> &range, BuildPair<link_ctn> &links, Data &data) = 0;

  static std::string serialize(const Query &query, subset_ctn &subsets, const RangePivot &root);

 protected:
  static void restrict(range_ctn &range, range_ctn &response, const subset_t &subset, CopyOption &option);

  template<typename _Aggr>
  static void group_by_subset(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                              range_ctn &range, const subset_pivot_ctn &subset);

  template<typename _Aggr>
  static void group_by_range(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                             range_ctn &range, const subset_pivot_ctn &subset);

  template<typename _Aggr>
  static void group_by_none(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                            range_ctn &range, const subset_pivot_ctn &subset);

  template<typename _Aggr>
  static void group_by_none(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                            range_ctn &range);

  static inline bool search_iterators(range_it &it_range, const range_ctn &range,
                                      pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset);

  static inline void compactation(range_ctn &input, range_ctn &output, CopyOption option);

  static inline void swap_and_sort(range_ctn &range, range_ctn &response, CopyOption option);

  const DimensionSchema _schema;
};

template<typename _Aggr>
void Dimension::group_by_subset(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                                range_ctn &range, const subset_pivot_ctn &subset) {

  _Aggr aggregator(subset.size());

  for (auto el = 0; el < subset.size(); ++el) {
    pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
      aggregator.merge(el, it_lower, it_upper);
      ++it_range;
    }

    aggregator.output(el, subset[el]->value, clausule, writer);
  }
}

template<typename _Aggr>
void Dimension::group_by_range(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                               range_ctn &range, const subset_pivot_ctn &subset) {

  _Aggr aggregator;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      aggregator.merge((*it_range).value, it_lower, it_upper);
      ++it_range;
    }
  }

  aggregator.output(clausule, writer);
}

template<typename _Aggr>
void Dimension::group_by_none(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                              range_ctn &range, const subset_pivot_ctn &subset) {

  _Aggr aggregator;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_it it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      aggregator.merge(it_lower, it_upper);
      ++it_range;
    }
  }

  aggregator.output(clausule, writer);
}

template<typename _Aggr>
void Dimension::group_by_none(const Query::clausule &clausule, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                              range_ctn &range) {

  _Aggr aggregator;

  aggregator.merge(range.begin(), range.end());

  aggregator.output(clausule, writer);
}

bool Dimension::search_iterators(range_it &it_range, const range_ctn &range,
                                 pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset) {
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

void Dimension::compactation(range_ctn &input, range_ctn &output, CopyOption option) {
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

void Dimension::swap_and_sort(range_ctn &range, range_ctn &response, CopyOption option) {
  // according to benchmark, this is a bit slower than std::sort() on randomized
  // sequences,  but much faster on partially - sorted sequences
  gfx::timsort(response.begin(), response.end());

  range.clear();

  // compaction (and swap) phase
  compactation(response, range, option);

  response.clear();
}
