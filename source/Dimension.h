#pragma once

#include "BinnedPivot.h"
#include "Data.h"
#include "Pivot.h"
#include "Query.h"
#include "Aggr.h"

class NDS;

class Dimension {
 public:
  enum Type { Spatial, Temporal, Categorical };

  Dimension(const std::tuple<uint32_t, uint32_t, uint32_t> &tuple);
  virtual ~Dimension() = default;

  virtual bool query(const Query &query, subset_container &subsets) const = 0;

  virtual uint32_t build(const build_ctn &range, build_ctn &response,
                         const link_ctn &links, link_ctn &share, NDS &nds) = 0;

  static std::string serialize(const Query &query, subset_container &subsets,
                               const BinnedPivot &root);

 protected:
  static void restrict(range_container &range, range_container &response,
                       const subset_t &subset, CopyOption &option);

  template<typename _Aggr>
  static void write_subset(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                           range_container &range, const binned_ctn &subset);

  template<typename _Aggr>
  static void write_range(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                          range_container &range, const binned_ctn &subset);

  static void write_none(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                         range_container &range, const binned_ctn &subset);

  static inline bool search_iterators(range_iterator &it_range, const range_container &range,
                                      pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset);

  static inline void compactation(range_container &input, range_container &output, CopyOption option);

  static inline void swap_and_sort(range_container &range, range_container &response, CopyOption option);

  static inline uint32_t aggregate_count(pivot_it &it_lower, pivot_it &it_upper);

  const uint32_t _key, _bin, _offset;
};

template<typename _Aggr>
void Dimension::write_subset(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                             range_container &range, const binned_ctn &subset) {

  _Aggr aggregator(subset.size());

  for (auto el = 0; el < subset.size(); ++el) {
    pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
    range_iterator it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
      aggregator.merge(el, it_lower, it_upper);
      ++it_range;
    }

    aggregator.output(el, subset[el]->value, query, writer);
  }
}

template<typename _Aggr>
void Dimension::write_range(const Query &query, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                            range_container &range, const binned_ctn &subset) {

  _Aggr aggregator;

  for (const auto &el : subset) {
    pivot_it it_lower = el->ptr().begin(), it_upper;
    range_iterator it_range = range.begin();

    while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
      aggregator.merge((*it_range).value, it_lower, it_upper);
      ++it_range;
    }
  }

  aggregator.output(query, writer);
}

bool Dimension::search_iterators(range_iterator &it_range, const range_container &range,
                                 pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset) {
  if (it_lower == subset.end() || it_range == range.end()) return false;

  if ((*it_range).pivot.ends_before(*it_lower)) {
    // binary search to find range iterator
    it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);

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

void Dimension::compactation(range_container &input, range_container &output, CopyOption option) {
  output.emplace_back(input.front());

  if (option == DefaultCopy || option == CopyValueFromSubset) {
    for (size_t i = 1; i < input.size(); ++i) {
      if (Pivot::is_sequence(output.back().pivot, input[i].pivot)) {
        output.back().pivot.merge_pivot(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  } else {
    for (size_t i = 1; i < input.size(); ++i) {
      if (BinnedPivot::is_sequence(output.back(), input[i])) {
        output.back().pivot.merge_pivot(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  }
}

void Dimension::swap_and_sort(range_container &range, range_container &response, CopyOption option) {
  // according to benchmark, this is a bit slower than std::sort() on randomized
  // sequences,  but much faster on partially - sorted sequences
  gfx::timsort(response.begin(), response.end());

  range.clear();

  // compaction (and swap) phase
  compactation(response, range, option);

  response.clear();
}

uint32_t Dimension::aggregate_count(pivot_it &it_lower, pivot_it &it_upper) {
  uint32_t count = 0;
  while (it_lower != it_upper) {
    count += (*it_lower++).size();
  }
  return count;
}
