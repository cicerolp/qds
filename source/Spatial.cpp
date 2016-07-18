#include "stdafx.h"
#include "Spatial.h"

Spatial::Spatial(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   for (const auto& ptr : range) {
      pivots_count += _container.build(ptr, data);
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
   std::cout << "size: " << subset.size() << std::endl;
   std::cout << "summed size: " << count << std::endl;

   for (const auto& r : range) {
      for (const auto& el : subset) {

         const auto it_lower = std::lower_bound(el->pivots().begin(), el->pivots().end(), r.pivot, Pivot::lower_bound_comp);
         const auto it_upper = std::upper_bound(it_lower, el->pivots().end(), r.pivot, Pivot::upper_bound_comp);
         
         // case 0
         /*response.insert(response.end(), it_lower, it_upper);*/

         // case 1
         /*const uint64_t& rvalue = r.value;
         std::transform(it_lower, it_upper, std::back_inserter(response), [rvalue](const Pivot& p) { return BinnedPivot(p, rvalue); });*/

         /*response.insert(response.end(), it_lower, it_upper);
         const uint64_t& rvalue = r.value;
         std::for_each(response.end() - (it_upper - it_lower), response.end(), [rvalue](BinnedPivot& p) { p.value = rvalue; });*/

         // case 2
         /*const uint64_t& rvalue = el->tile();
         std::transform(it_lower, it_upper, std::back_inserter(response), [rvalue](const Pivot& p) { return BinnedPivot(p, rvalue); });*/

         response.insert(response.end(), it_lower, it_upper);
         const uint64_t& rvalue = el->tile();
         std::for_each(response.end() - (it_upper - it_lower), response.end(), [rvalue](BinnedPivot& p) { p.value = rvalue; });

         /*for (auto it = it_lower; it != it_upper; ++it) {
            response.emplace_back((*it), el->tile());

            /*BUG remove comment#1#
            //if (r.pivot.endsWith(*it)) break;
         }*/

         // BUG remove comment
         //if (r.pivot.endsWith(*it)) break;
      }
   }

   return true;
}
