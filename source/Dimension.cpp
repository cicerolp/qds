#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

void Dimension::restrict(range_container& range, response_container& response, const binned_container& subset, CopyOption option) {
   // sort range only when necessary
   swap_and_sort(range, response);

   if (range.size() == 0) return;

   pivot_iterator it_lower, it_upper;

   for (const auto& el : subset) {

      it_lower = el->pivots.begin();

      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {

         if (!search_iterators(it_range, range, it_lower, it_upper, el->pivots)) break;

         switch (option) {
            case CopyValueFromRange:
               while (it_lower != it_upper) {
                  response.emplace_back((*it_lower++), (*it_range).value);
               }
               break;
            case CopyValueFromSubset:               
               while (it_lower != it_upper) {
                  response.emplace_back((*it_lower++), el->value);
               }
               break;
            default: // DefaultCopy
               response.insert(response.end(), it_lower, it_upper);
               it_lower = it_upper;
               break;
         }
      }
   }
}
