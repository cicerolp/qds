#pragma once

#include "Pivot.h"

class BinnedPivot {
public:
   BinnedPivot() = default;

   BinnedPivot(const Pivot& pivot)
      : pivot(pivot) {}

   BinnedPivot(const Pivot& pivot, const uint64_t& value)
      : pivot(pivot), value(value) {}

   BinnedPivot(const BinnedPivot& other) = default;
   BinnedPivot(BinnedPivot&& other) = default;
   BinnedPivot& operator=(const BinnedPivot& other) = default;
   BinnedPivot& operator=(BinnedPivot&& other) = default;

   inline bool operator< (const BinnedPivot& other) const {
      return pivot < other.pivot;
   }

   static inline bool is_sequence(const BinnedPivot& lhs, const BinnedPivot& rhs) {
      return Pivot::is_sequence(lhs.pivot, rhs.pivot) && lhs.value == rhs.value;
   }

   static inline bool upper_bound_comp(const BinnedPivot& lhs, const Pivot& rhs) {
      return Pivot::upper_bound_comp(lhs.pivot, rhs);
   }

   Pivot pivot;
   uint64_t value;
};

using range_container = std::vector<BinnedPivot>;
using range_iterator = range_container::const_iterator;
