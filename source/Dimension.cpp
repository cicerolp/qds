#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

void Dimension::restrict(range_container& range, response_container& response, binned_container& subset, const CopyOption option) {
   // sort range only when necessary
   swap_and_sort(range, response);

   if (range.size() == 0) return;

   pivot_iterator iters_it;
   pivot_iterator it_lower;

   for (const auto& el : subset) {

      iters_it = el->pivots.begin();

      for (const auto& r : range) {

         if (iters_it == el->pivots.end()) {
            break;

         } else if (r.pivot.begins_after(*iters_it)) {
            it_lower = std::lower_bound(iters_it, el->pivots.end(), r.pivot, Pivot::lower_bound_comp);
            if (it_lower == el->pivots.end()) break;

         } else if (r.pivot.ends_before(*iters_it)) {
            continue;

         } else {
            it_lower = iters_it;
         }

         switch (option) {
            case CopyValueFromRange:
               while (it_lower != el->pivots.end() && r.pivot >= (*it_lower)) {
                  response.emplace_back((*it_lower), r.value);
                  it_lower++;
               }
               break;
            case CopyValueFromSubset:
               while (it_lower != el->pivots.end() && r.pivot >= (*it_lower)) {
                  response.emplace_back((*it_lower), el->value);
                  it_lower++;
               }
               
               break;
            default:
               while (it_lower != el->pivots.end() && r.pivot >= (*it_lower)) {
                  it_lower++;
               }
               response.insert(response.end(), iters_it, it_lower);
               break;
         }

         iters_it = it_lower;
      }
   }

   // clear subset container
   subset.clear();
}
