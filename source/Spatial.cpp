#include "stdafx.h"
#include "Spatial.h"

Spatial::Spatial(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   for (const auto& ptr : range) {
      pivots_count += _container.build(ptr, data, 0);
   }

   return pivots_count;
}

bool Spatial::query(const Query& query, const response_container& range, response_container& response) const {

   std::vector<const SpatialElement*> subset;
   _container.query(query, subset);

   for (const auto& r : range) {
      for (const auto& el : subset) {

         const auto pivot_it = std::lower_bound(el->pivots().begin(), el->pivots().end(), r.pivot);

         if (pivot_it != el->pivots().end() && r.pivot.contains(*pivot_it)) {
            response.emplace_back((*pivot_it), el->tile());

            if (r.pivot.endsWith(*pivot_it)) break;
         }
      }
   }

   return true;
}
