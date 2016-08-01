#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile) : value(tile) { }

SpatialElement::SpatialElement(const spatial_t& tile, const building_container& container)
   : value(tile), pivots(container.size()) {
   std::memcpy(&pivots[0], &container[0], container.size() * sizeof(Pivot));
}

uint32_t SpatialElement::expand(Data& data, const uint8_t offset) {

   uint32_t pivots_count = static_cast<uint32_t>(pivots.size());
   uint8_t next_level = value.z + 1;

   if (next_level < max_levels && count_expand()) {

      std::map<spatial_t, building_container> tmp_container;

      // node will be expanded
      value.leaf = 0;

      for (const auto& ptr : pivots) {
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

            tmp_container[pair.first].emplace_back(first, second);
         }
      }

      for (auto& pair : tmp_container) {
         _container[pair.first] = std::make_unique<SpatialElement>(pair.first, pair.second);
      }

      for (auto& el : _container) {
         if (el != nullptr) {
            pivots_count += el->expand(data, offset);
         }
      }
   }

   return pivots_count;
}

void SpatialElement::query_tile(const Query& query, std::vector<const SpatialElement*>& subset, uint8_t z) const {
   // BUG fix spatial at   
   if (value.contains(query.tile(0))) {
      if (z == query.tile(0).z || value.leaf) {
         return aggregate_tile(query, subset, z);
      } else {
         if (_container[0] != nullptr) _container[0]->query_tile(query, subset, z + 1);
         if (_container[1] != nullptr) _container[1]->query_tile(query, subset, z + 1);
         if (_container[2] != nullptr) _container[2]->query_tile(query, subset, z + 1);
         if (_container[3] != nullptr) _container[3]->query_tile(query, subset, z + 1);
      }
   }
}

void SpatialElement::query_region(const Query& query, std::vector<const SpatialElement*>& subset, uint8_t z) const {
   // BUG fix spatial at
   if ((z == query.region(0).z || value.leaf) && query.region(0).intersect(value, z)) {
      subset.emplace_back(this);
   } else if (z < query.region(0).z) {
      if (_container[0] != nullptr) _container[0]->query_region(query, subset, z + 1);
      if (_container[1] != nullptr) _container[1]->query_region(query, subset, z + 1);
      if (_container[2] != nullptr) _container[2]->query_region(query, subset, z + 1);
      if (_container[3] != nullptr) _container[3]->query_region(query, subset, z + 1);
   }
}

void SpatialElement::aggregate_tile(const Query& query, std::vector<const SpatialElement*>& subset, uint8_t z) const {
   // BUG fix spatial at
   if (value.leaf || (z == query.tile(0).z + query.resolution())) {
      subset.emplace_back(this);
   } else {
      if (_container[0] != nullptr) _container[0]->aggregate_tile(query, subset, z + 1);
      if (_container[1] != nullptr) _container[1]->aggregate_tile(query, subset, z + 1);
      if (_container[2] != nullptr) _container[2]->aggregate_tile(query, subset, z + 1);
      if (_container[3] != nullptr) _container[3]->aggregate_tile(query, subset, z + 1);
   }
}
