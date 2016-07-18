#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile)
   : _tile(tile) {
   _tile.leaf = 1;
}

uint32_t SpatialElement::build(const Pivot& range, Data& data, uint8_t zoom) {

   uint32_t pivots_count = 1;
   _pivots.emplace_back(range);

   // increment zoom
   zoom += 1;

   if (zoom < max_levels) {
      std::map<spatial_t, uint32_t> used;

      for (auto i = range.front(); i < range.back(); ++i) {
         coordinates_t value = data.record<coordinates_t>(i, 0 /*BUG fix offset*/);

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

         if (_container[pair.first] == nullptr && range.size() > 1) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
            pivots_count += _container[pair.first]->expand((*this), Pivot(first, second), data, zoom);

         } else if (_container[pair.first] != nullptr) {
            pivots_count += _container[pair.first]->build(Pivot(first, second), data, zoom);
         }

         /*if (_container[pair.first] == nullptr) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
         }
         pivots_count += _container[pair.first]->build(Pivot(first, second), data, zoom);*/
      }
   }  

   return pivots_count;
}

uint32_t SpatialElement::expand(SpatialElement& parent, const Pivot& pivot, Data& data, uint8_t zoom) {

   uint32_t pivots_count = 0;

   for (const auto& ptr : parent._pivots) {
      if (ptr.size() == 1) {

         coordinates_t value = data.record<coordinates_t>(ptr.front(), 0 /*BUG fix offset*/);

         auto y = mercator_util::lat2tiley(value.lat, zoom);
         auto x = mercator_util::lon2tilex(value.lon, zoom);

         if (spatial_t(x, y, zoom) == _tile) {
            _pivots.emplace_back(ptr);
            pivots_count += 1;
         }
      }
   }

   parent._tile.leaf = 0;

   return pivots_count + build(pivot, data, zoom);
}

void SpatialElement::query(const Query& query, std::vector<const SpatialElement*>& subset) const {

   /*BUG fix last node*/
   if (query.tile().second == _tile /* || (last() && util::intersects(_pivot.value(), query.getTile(d)))*/) {
      return aggregate_tile(query, subset);

   } else if (_tile.z < query.tile().second.z) {
      const uint32_t n = static_cast<uint32_t>(std::pow(2, query.tile().second.z - _tile.z));

      const uint32_t x0 = static_cast<uint32_t>(_tile.x) * n;
      const uint32_t x1 = x0 + n;

      const uint32_t y0 = static_cast<uint32_t>(_tile.y) * n;
      const uint32_t y1 = y0 + n;

      if (x0 <= query.tile().second.x && x1 >= query.tile().second.x && y0 <= query.tile().second.y && y1 >= query.tile().second.y) {
         if (_container[0] != nullptr) _container[0]->query(query, subset);
         if (_container[1] != nullptr) _container[1]->query(query, subset);
         if (_container[2] != nullptr) _container[2]->query(query, subset);
         if (_container[3] != nullptr) _container[3]->query(query, subset);
      }
   }
}

void SpatialElement::aggregate_tile(const Query& query, std::vector<const SpatialElement*>& subset) const {

   /*BUG fix last node*/
   if (_tile.leaf || (_tile.z == query.tile().second.z + query.resolution())) {
      subset.emplace_back(this);
   } else {
      if (_container[0] != nullptr) _container[0]->aggregate_tile(query, subset);
      if (_container[1] != nullptr) _container[1]->aggregate_tile(query, subset);
      if (_container[2] != nullptr) _container[2]->aggregate_tile(query, subset);
      if (_container[3] != nullptr) _container[3]->aggregate_tile(query, subset);
   }
}
