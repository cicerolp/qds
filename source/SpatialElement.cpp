#include "stdafx.h"
#include "SpatialElement.h"

SpatialElement::SpatialElement(const Pivot& pivot, const spatial_t& tile)
   : _pivot(pivot), _tile(tile) {}

uint32_t SpatialElement::build(building_container& response, Data& data, uint8_t zoom) {

   uint32_t pivots_count = 1;

   // BUG fix leaf
   if (zoom < 25 && _pivot.size() > 1) {

      // increment zoom
      zoom += 1;

      std::map<spatial_t, uint32_t> used;

      for (auto i = _pivot.front(); i < _pivot.back(); ++i) {
         // BUG fix offset
         coordinates_t value = data.record<coordinates_t>(i, 0);

         auto y = mercator_util::lat2tiley(value.lat, zoom);
         auto x = mercator_util::lon2tilex(value.lon, zoom);

         spatial_t tile(x, y, zoom);

         data.setHash(i, tile);
         used[tile]++;
      }

      // sorting
      data.sort(_pivot.front(), _pivot.back());

      uint32_t accum = _pivot.front();
      for (const auto& pair : used) {

         if (pair.second == 0) continue;

         uint32_t first = accum;
         accum += pair.second;
         uint32_t second = accum;

         _container[pair.first] = std::make_unique<SpatialElement>(Pivot(first, second), pair.first);
         pivots_count += _container[pair.first]->build(response, data, zoom);
      }
   } else {
      response.emplace_back(_pivot);
   }

   return pivots_count;
}


