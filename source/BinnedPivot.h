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
   };

   Pivot pivot;
   uint64_t value;
};

using response_container = std::vector<BinnedPivot>;
using response_iterator = response_container::const_iterator;
