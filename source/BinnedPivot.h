#pragma once

#include "Pivot.h"

class BinnedPivot {
public:
   BinnedPivot(const Pivot& pivot, const uint64_t& value)
      : pivot(pivot), value(value) {}
   
   const Pivot pivot;
   const uint64_t value;
};

using response_container = std::vector<BinnedPivot>;
using response_iterator = response_container::const_iterator;