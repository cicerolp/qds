#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile)
   : value(tile) { }

uint32_t SpatialElement::expand(Data& data, const uint8_t offset) {

   uint32_t pivots_count = static_cast<uint32_t>(pivots.size());
   uint8_t next_level = value.z + 1;

   if (next_level < max_levels && (pivots.size() > 1 || pivots.front().size() > 1)) {

      // node will be expanded
      value.leaf = 0;

      for(const auto& ptr : pivots) {
         std::map<spatial_t, uint32_t> used;

         for (auto i = ptr.front(); i < ptr.back(); ++i) {
            coordinates_t value = data.record<coordinates_t>(i, offset);

            auto y = mercator_util::lat2tiley(value.lat, next_level);
            auto x = mercator_util::lon2tilex(value.lon, next_level);

            spatial_t tile(x, y, next_level);

            data.setHash(i, tile);
            used[tile]++;
         }

         // sorting
         data.sort(ptr.front(), ptr.back());

         uint32_t accum = ptr.front();
         for (const auto& pair : used) {

            if (pair.second == 0) continue;

            uint32_t first = accum;
            accum += pair.second;
            uint32_t second = accum;

            if (_container[pair.first] == nullptr) {
               _container[pair.first] = std::make_unique<SpatialElement>(pair.first);
            }
            _container[pair.first]->add_element(first, second);
         }
      }

      for (auto& el : _container) {
         if (el != nullptr) {
            pivots_count += el->expand(data, offset);
         }
      }
   }

   return pivots_count;
}

void SpatialElement::query(const Query& query, std::vector<const SpatialElement*>& subset) const {

   if (query.tile().second == value || (value.leaf && value.intersects(query.tile().second))) {
      return aggregate_tile(query, subset);

   } else if (value.z < query.tile().second.z && value.contains(query.tile().second)) {
      if (_container[0] != nullptr) _container[0]->query(query, subset);
      if (_container[1] != nullptr) _container[1]->query(query, subset);
      if (_container[2] != nullptr) _container[2]->query(query, subset);
      if (_container[3] != nullptr) _container[3]->query(query, subset);
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
