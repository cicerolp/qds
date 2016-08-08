#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile) {
   el.value = tile;
}

SpatialElement::SpatialElement(const spatial_t& tile, const building_container& container) {
   el.value = tile.data;
   set_range(container);
}

uint32_t SpatialElement::expand(Data& data, building_container& response, const uint8_t offset) {

   uint32_t pivots_count = static_cast<uint32_t>(el.pivots.size());
   uint8_t next_level = (*(spatial_t*)&el.value).z + 1;

   if (next_level < max_levels && count_expand()) {

      std::map<spatial_t, building_container> tmp_container;

      // node will be expanded
      (*(spatial_t*)&el.value).leaf = 0;

      for (const auto& ptr : el.pivots) {
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

      for (auto& binned : _container) {
         if (binned != nullptr) {
            pivots_count += binned->expand(data, response, offset);
         }
      }
   } else {
      response.insert(response.end(), el.pivots.begin(), el.pivots.end());
   }

   return pivots_count;
}

void SpatialElement::query_tile(const std::vector<spatial_t>& tile, uint8_t resolution, binned_container& subset, uint8_t z) const {
   // BUG fix spatial at   
   if ((*(spatial_t*)&el.value).contains(tile[0])) {
      if (z == tile[0].z || (*(spatial_t*)&el.value).leaf) {
         return aggregate_tile(tile, resolution, subset, z);
      } else {
         if (_container[0] != nullptr) _container[0]->query_tile(tile, resolution, subset, z + 1);
         if (_container[1] != nullptr) _container[1]->query_tile(tile, resolution, subset, z + 1);
         if (_container[2] != nullptr) _container[2]->query_tile(tile, resolution, subset, z + 1);
         if (_container[3] != nullptr) _container[3]->query_tile(tile, resolution, subset, z + 1);
      }
   }
}

void SpatialElement::query_region(const std::vector<region_t>& region, binned_container& subset, uint8_t z) const {
   // BUG fix spatial at

   if (z <= region[0].z) {
      if ((z == region[0].z || (*(spatial_t*)&el.value).leaf)) {
         if (region[0].intersect((*(spatial_t*)&el.value), z)) {
            subset.emplace_back(&el);
         } else {
            return;
         }
      } else if(region[0].equals((*(spatial_t*)&el.value), z)) {
         subset.emplace_back(&el);
      } else {
         if (_container[0] != nullptr) _container[0]->query_region(region, subset, z + 1);
         if (_container[1] != nullptr) _container[1]->query_region(region, subset, z + 1);
         if (_container[2] != nullptr) _container[2]->query_region(region, subset, z + 1);
         if (_container[3] != nullptr) _container[3]->query_region(region, subset, z + 1);
      }
   }

   /*if ((z == region[0].z || (*(spatial_t*)&el.value).leaf) && region[0].intersect((*(spatial_t*)&el.value), z)) {
      subset.emplace_back(&el);
   } else if (z < region[0].z) {
      if (_container[0] != nullptr) _container[0]->query_region(region, subset, z + 1);
      if (_container[1] != nullptr) _container[1]->query_region(region, subset, z + 1);
      if (_container[2] != nullptr) _container[2]->query_region(region, subset, z + 1);
      if (_container[3] != nullptr) _container[3]->query_region(region, subset, z + 1);
   }*/
}

void SpatialElement::aggregate_tile(const std::vector<spatial_t>& tile, uint8_t resolution, binned_container& subset, uint8_t z) const {
   // BUG fix spatial at
   if ((*(spatial_t*)&el.value).leaf || (z == tile[0].z + resolution)) {
      subset.emplace_back(&el);
   } else {
      if (_container[0] != nullptr) _container[0]->aggregate_tile(tile, resolution, subset, z + 1);
      if (_container[1] != nullptr) _container[1]->aggregate_tile(tile, resolution, subset, z + 1);
      if (_container[2] != nullptr) _container[2]->aggregate_tile(tile, resolution, subset, z + 1);
      if (_container[3] != nullptr) _container[3]->aggregate_tile(tile, resolution, subset, z + 1);
   }
}
