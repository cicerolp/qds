#include "stdafx.h"
#include "Spatial.h"

Spatial::Spatial(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   for (const auto& ptr : range) {
      pivots_count += _container.build(ptr, data, _offset);
   }

   return pivots_count;
}

bool Spatial::query(const Query& query, const response_container& range, response_container& response) const {

   if (query.tile().first != _key) return false;

   std::vector<const SpatialElement*> subset;
   _container.query(query, subset);

   int count = 0;
   for (const auto& el : subset) {
      for (auto ptr : el->pivots) {
         count += ptr.size();
      }
   }

   std::cout <<  "summed value:  " << count << std::endl
   ;

   for (const auto& r : range) {
      for (const auto& el : subset) {

         auto it_lower = std::lower_bound(el->pivots.begin(), el->pivots.end(), r.pivot, Pivot::lower_bound_comp);
         auto it_upper = std::upper_bound(it_lower, el->pivots.end(), r.pivot, Pivot::upper_bound_comp);
         
         // case 0
         /*response.insert(response.end(), it_lower, it_upper);*/

         // case 1
         /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, r.value); });*/

         /*response.insert(response.end(), it_lower, it_upper);
         std::for_each(response.end() - (it_upper - it_lower), response.end(), [&](BinnedPivot& p) { p.value = r.value; });*/

         // case 2
         //std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, el->value.data); });

         response.insert(response.end(), it_lower, it_upper);
         std::for_each(response.end() - (it_upper - it_lower), response.end(), [&](BinnedPivot& p) { p.value = el->value.data; });

         if (r.pivot.endsWith(*(--it_upper))) break;
      }
   }

   return true;
}
