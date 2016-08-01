#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
      : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)){ }
	virtual ~Dimension() = default;

   virtual bool query(const Query& query, range_container& range, response_container& response, binned_container& subset, bool& pass_over_target) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

protected:
   enum CopyOption {
      CopyValueFromRange, CopyValueFromSubset, DefaultCopy
   };

   static void restrict(range_container& range, response_container& response, binned_container& subset, const CopyOption option);

   static inline void swap_and_sort(range_container& range, response_container& response) {
      // vector, vector
      range.swap(response);
      std::sort(range.begin(), range.end());
      response.clear();
   }

   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};