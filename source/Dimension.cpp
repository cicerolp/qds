#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

void Dimension::restrict(range_container& range, response_container& response, binned_container& subset, const CopyOption option) {
   // sort range only when necessary
   swap_and_sort(range, response);

   if (range.size() == 0) return;

   pivot_iterator it_lower;
   pivot_iterator it_upper;

   for (const auto& el : subset) {

      it_lower = el->pivots.begin();

      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {

         if (it_lower == el->pivots.end()) break;

      search:
         if ((*it_range).pivot.begins_after(*it_lower)) {
            // binnary search to find subset iterator
            it_upper = std::lower_bound(it_lower, el->pivots.end(), (*it_range).pivot, Pivot::lower_bound_comp);
            if (it_upper == el->pivots.end()) break;

         } else if ((*it_range).pivot.ends_before(*it_lower)) {
            // binary search to find range iterator
            it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);

            if (it_range != range.end()) {
               goto search;
            } else {
               break;
            }
         } else {
            it_upper = it_lower;
         }

         switch (option) {
            case CopyValueFromRange:
               while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                  response.emplace_back((*it_upper), (*it_range).value);
                  ++it_upper;
               }
               break;
            case CopyValueFromSubset:
               while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                  response.emplace_back((*it_upper), el->value);
                  ++it_upper;
               }
               break;
            default:
               while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                  ++it_upper;
               }
               response.insert(response.end(), it_lower, it_upper);
               break;
         }

         it_lower = it_upper;
      }
   }

   // clear subset container
   subset.clear();
}
