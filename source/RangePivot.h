#pragma once

#include "Pivot.h"

class RangePivot {
 public:
  RangePivot() = default;

  RangePivot(const Pivot& pivot) : pivot(pivot) {}

  RangePivot(const Pivot& pivot, const uint64_t& value)
      : pivot(pivot), value(value) {}

  RangePivot(const RangePivot& other) = default;
  RangePivot(RangePivot&& other) = default;
  RangePivot& operator=(const RangePivot& other) = default;
  RangePivot& operator=(RangePivot&& other) = default;

  inline bool operator<(const RangePivot& other) const {
    return pivot < other.pivot;
  }

  static inline bool is_sequence(const RangePivot& lhs,
                                 const RangePivot& rhs) {
    return Pivot::is_sequence(lhs.pivot, rhs.pivot) && lhs.value == rhs.value;
  }

  static inline bool upper_bound_comp(const RangePivot& lhs,
                                      const Pivot& rhs) {
    return Pivot::upper_bound_comp(lhs.pivot, rhs);
  }

  Pivot pivot;
  uint64_t value;
};

using range_ctn = std::vector<RangePivot>;
using range_it = range_ctn::const_iterator;
