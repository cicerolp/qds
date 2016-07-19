#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile)
   : value(tile) { }

uint32_t SpatialElement::expand(Data& data, const uint8_t offset) {

   if (value.leaf == 0) return 0;

   uint32_t pivots_count = 0;
   uint8_t next_level = value.z + 1;

   std::map<spatial_t, std::vector<Pivot>> used;
   for (const auto& ptr : pivots) {
      if (ptr.size() == 1) {
         coordinates_t value = data.record<coordinates_t>(ptr.front(), offset);

         auto y = mercator_util::lat2tiley(value.lat, next_level);
         auto x = mercator_util::lon2tilex(value.lon, next_level);

         spatial_t tile(x, y, next_level);
         if (_container[tile] == nullptr) {
            used[tile].emplace_back(ptr);
         }
      }
   }

   for (const auto& pair : used) {
      _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
      _container[pair.first]->pivots.insert(_container[pair.first]->pivots.end(), pair.second.begin(), pair.second.end());

      pivots_count += static_cast<uint32_t>(pair.second.size());
   }

   value.leaf = 0;
   return pivots_count;
}

uint32_t SpatialElement::build(const Pivot& pivot, Data& data, const uint8_t offset) {

   uint32_t pivots_count = 1;
   uint8_t next_level = value.z + 1;

   if (next_level < max_levels) {

      // first expand, then add current pivot
      if (pivots.size() > 0) expand(data, offset);

      pivots.emplace_back(pivot);

      std::map<spatial_t, uint32_t> used;

      for (auto i = pivot.front(); i < pivot.back(); ++i) {
         coordinates_t value = data.record<coordinates_t>(i, offset);

         auto y = mercator_util::lat2tiley(value.lat, next_level);
         auto x = mercator_util::lon2tilex(value.lon, next_level);

         spatial_t tile(x, y, next_level);

         data.setHash(i, tile);
         used[tile]++;
      }

      // sorting
      data.sort(pivot.front(), pivot.back());

      uint32_t accum = pivot.front();
      for (const auto& pair : used) {

         if (pair.second == 0) continue;

         uint32_t first = accum;
         accum += pair.second;
         uint32_t second = accum;

         /*if (_container[pair.first] == nullptr) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
         }
         pivots_count += _container[pair.first]->build(Pivot(first, second), data);*/

         if (_container[pair.first] == nullptr && ((second - first) > 1 || pivots.size() > 1 || pivots.front().size() > 1)) {
            _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
         }

         if (_container[pair.first] != nullptr) {
            pivots_count += _container[pair.first]->build(Pivot(first, second), data, offset);
         }
      }
   } else {
      pivots.emplace_back(pivot);
   }

   return pivots_count;
}

void SpatialElement::query(const Query& query, std::vector<const SpatialElement*>& subset) const {

   /*BUG fix last node*/
   if (query.tile().second == value /* || (last() && util::intersects(_pivot.value(), query.getTile(d)))*/) {
      return aggregate_tile(query, subset);

   } else if (value.z < query.tile().second.z) {
      const uint32_t n = static_cast<uint32_t>(std::pow(2, query.tile().second.z - value.z));

      const uint32_t x0 = static_cast<uint32_t>(value.x) * n;
      const uint32_t x1 = x0 + n;

      const uint32_t y0 = static_cast<uint32_t>(value.y) * n;
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

   if (value.leaf || (value.z == query.tile().second.z + query.resolution())) {
      subset.emplace_back(this);
   } else {
      if (_container[0] != nullptr) _container[0]->aggregate_tile(query, subset);
      if (_container[1] != nullptr) _container[1]->aggregate_tile(query, subset);
      if (_container[2] != nullptr) _container[2]->aggregate_tile(query, subset);
      if (_container[3] != nullptr) _container[3]->aggregate_tile(query, subset);
   }
}
