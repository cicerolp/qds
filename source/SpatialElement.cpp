#include "stdafx.h"
#include "SpatialElement.h"

SpatialElement::SpatialElement(const spatial_t& tile)
   : _tile(tile) {}

uint32_t SpatialElement::build(const Pivot& range, building_container& response, Data& data, uint8_t zoom) {

   uint32_t pivots_count = 1;
   _pivots.emplace_back(range);

   // BUG fix leaf
   if (zoom < 25 && range.size() > 1) {

      std::map<spatial_t, uint32_t> used;

      for (auto i = range.front(); i < range.back(); ++i) {
         // BUG fix offset
         coordinates_t value = data.record<coordinates_t>(i, 0);

         auto y = mercator_util::lat2tiley(value.lat, zoom);
         auto x = mercator_util::lon2tilex(value.lon, zoom);

         spatial_t tile(x, y, zoom);

         data.setHash(i, tile);
         used[tile]++;
      }

      // sorting
      data.sort(range.front(), range.back());

      uint32_t accum = range.front();
      for (const auto& pair : used) {

         if (pair.second == 0) continue;

         uint32_t first = accum;
         accum += pair.second;
         uint32_t second = accum;

         if (_container[pair.first] == nullptr) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);

            pivots_count += _container[pair.first]->expand(_pivots, Pivot(first, second), response, data, zoom);
         } else {
            pivots_count += _container[pair.first]->build(Pivot(first, second), response, data, zoom + 1);
         }
         
         /*if (_container[pair.first] == nullptr) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
         }
         pivots_count += _container[pair.first]->build(Pivot(first, second), response, data, zoom + 1);*/
         
      }
   }
   else {
      response.emplace_back(range);
   }

   return pivots_count;
}

uint32_t SpatialElement::expand(building_container& parent, const Pivot& pivot, building_container& response, Data& data, uint8_t zoom) {

   uint32_t pivots_count = 0;

   // algum deles vai para mim
   for (const auto& ptr : parent) {
      if (ptr.size() == 1) {
         // BUG fix offset
         coordinates_t value = data.record<coordinates_t>(ptr.front(), 0);

         auto y = mercator_util::lat2tiley(value.lat, zoom);
         auto x = mercator_util::lon2tilex(value.lon, zoom);

         spatial_t tile(x, y, zoom);

         if (tile == _tile) {
            _pivots.emplace_back(ptr);

            pivots_count += 1;
         }
      }
   }

   // coloca o pivot
   // chama build do pivot
   return pivots_count + build(pivot, response, data, zoom + 1);
}

