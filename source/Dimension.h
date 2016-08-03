#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   enum CopyOption {
      CopyValueFromRange,
      CopyValueFromSubset,
      DefaultCopy
   };

   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
      : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)) { }

   virtual ~Dimension() = default;

   virtual bool query(const Query& query, range_container& range, response_container& response, CopyOption& option) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

protected:
   static void restrict(range_container& range, response_container& response, const binned_container& subset, CopyOption option);

   virtual std::string serialize(const Query& query, range_container& range, binned_container& subset) const {
      return std::string();
   };

   static inline bool search_iterators(range_iterator& it_range, const range_container& range,
                                       pivot_iterator& it_lower, pivot_iterator& it_upper, const pivot_container& subset) {
      if (it_lower == subset.end()) return false;

      if ((*it_range).pivot.ends_before(*it_lower)) {
         // binary search to find range iterator
         it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);
         if (it_range == range.end()) return false;
      }

      if ((*it_range).pivot.begins_after(*it_lower)) {
         // binnary search to find lower subset iterator
         it_upper = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);
         if (it_upper == subset.end()) return false;
         it_lower = it_upper;
      }

      // binnary search to find upper subset iterator
      it_upper = std::upper_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::upper_bound_comp);

      return true;
   }

   static inline void swap_and_sort(range_container& range, response_container& response) {
      range.swap(response);
      std::sort(range.begin(), range.end());
      response.clear();
   }

   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};
