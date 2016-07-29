#include "stdafx.h"
#include "NDS.h"
#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   _container.add_range(range);
   pivots_count += _container.expand(data, _offset);

   return pivots_count;
}

bool Spatial::query(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const {

   std::vector<const SpatialElement*> subset;

   if (query.eval_tile(_key)) {
      _container.query_tile(query, subset);
   } else if (query.eval_region(_key)) {
      _container.query_region(query, subset, 0);
   } else {
      return false;
   }

   // sort range only when necessary
   NDS::swap_and_sort(range, response);

   for (const auto& el : subset) {

      auto iters_it = el->pivots.begin();

      for (const auto& r : range) {

         if (iters_it == el->pivots.end()) break;
         else if (!r.pivot.intersect_range((*iters_it), el->pivots.back())) continue;

         building_iterator it_lower = std::lower_bound(iters_it, el->pivots.end(), r.pivot, Pivot::lower_bound_comp);

         if (it_lower == el->pivots.end()) continue;

         while (it_lower != el->pivots.end() && r.pivot >= (*it_lower)) {
            if (pass_over_target) {
               // case 1
               response.emplace_back((*it_lower), r.value);
            } else if (query.type() == Query::TILE) {
               // case 2
               response.emplace_back((*it_lower), el->value.data);
            } else {
               // TODO optimize case 0
               // case 0
               response.emplace_back((*it_lower));
            }
            it_lower++;
         }

         iters_it = it_lower;
      }
   }

   if (query.type() == Query::TILE) {
      pass_over_target = true;
   }

   return true;
}
