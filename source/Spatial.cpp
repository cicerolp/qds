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

   if (query.tile().first != _key) return false;

   std::vector<const SpatialElement*> subset;
   _container.query(query, subset);

   int count = 0;
   for (const auto& el : subset) {
      for (const auto& r : el->pivots()) {
         count += r.size();
      }      
   }
   std::cout << "summed size: " << count << std::endl;

   for (const auto& r : range) {
      for (const auto& el : subset) {

         const auto it_lower = std::lower_bound(el->pivots().begin(), el->pivots().end(), r.pivot, Pivot::lower_bound_comp);
         const auto it_upper = std::upper_bound(it_lower, el->pivots().end(), r.pivot, Pivot::upper_bound_comp);
         
         for (auto it = it_lower; it != it_upper; ++it) {
            response.emplace_back((*it), el->tile());

            /*BUG remove comment*/
            //if (r.pivot.endsWith(*it)) break;
         }
      }
   }

   return true;
}
