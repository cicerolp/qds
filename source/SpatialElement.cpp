#include "stdafx.h"
#include "SpatialElement.h"

#include "mercator_util.h"

SpatialElement::SpatialElement(const spatial_t& tile) {
   el.value = tile.data;
}

SpatialElement::SpatialElement(const spatial_t& tile, const building_container& container) {
   el.value = tile.data;
   set_range(container);
}

uint32_t SpatialElement::expand(Data& data, building_container& response, uint32_t bin) {

   spatial_t& value = (*reinterpret_cast<spatial_t*>(&el.value));

   uint8_t next_level = value.z + 1;
   uint32_t pivots_count = static_cast<uint32_t>(el.pivots.size());

   if (next_level < max_levels && count_expand(bin)) {

      std::vector<building_container> tmp_container(4);

      // node will be expanded
      value.leaf = 0;

      for (const auto& ptr : el.pivots) {
         std::vector<uint32_t> used(4, 0);

         for (auto i = ptr.front(); i < ptr.back(); ++i) {
            coordinates_t coords = data.record<coordinates_t>(i);

            auto y = mercator_util::lat2tiley(coords.lat, next_level);
            auto x = mercator_util::lon2tilex(coords.lon, next_level);
            auto index = mercator_util::index(x, y);

            data.setHash(i, index);
            ++used[index];
         }

         // sorting
         data.sort(ptr.front(), ptr.back());

         uint32_t accum = ptr.front();
         for (int i = 0; i < 4; ++i) {
            if (used[i] == 0) continue;

            uint32_t first = accum;
            accum += used[i];
            uint32_t second = accum;

            tmp_container[i].emplace_back(first, second);
         }
      }

      for (int i = 0; i < 4; ++i) {
         if (tmp_container[i].size() == 0) continue;

         uint32_t x = value.x * 2;
         uint32_t y = value.y * 2;

         if (i == 1) {
            ++y;
         } else if (i == 2) {
            ++x;
         } else if (i == 3) {
            ++y;
            ++x;
         }

         _container[i] = std::make_unique<SpatialElement>(spatial_t(x, y, next_level), tmp_container[i]);
         pivots_count += _container[i]->expand(data, response, bin);
      }
   } else {
      response.insert(response.end(), el.pivots.begin(), el.pivots.end());
   }

   return pivots_count;
}

void SpatialElement::query_tile(const spatial_t& tile, uint64_t resolution, binned_container& subset, uint64_t zoom) const {
   const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

   if (value.contains(tile)) {
      if (value.leaf || zoom == tile.z) {
         return aggregate_tile(tile.z + resolution, subset, zoom);
      } else {
         if (_container[0] != nullptr) _container[0]->query_tile(tile, resolution, subset, zoom + 1);
         if (_container[1] != nullptr) _container[1]->query_tile(tile, resolution, subset, zoom + 1);
         if (_container[2] != nullptr) _container[2]->query_tile(tile, resolution, subset, zoom + 1);
         if (_container[3] != nullptr) _container[3]->query_tile(tile, resolution, subset, zoom + 1);
      }
   }
}

void SpatialElement::query_region(const region_t& region, binned_container& subset, uint64_t zoom) const {
   const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

   if (region.intersect(value)) {
      if (zoom == region.z || value.leaf) {
         subset.emplace_back(&el);
      } else if(region.cover(value)) {
         subset.emplace_back(&el);
      } else {
         if (_container[0] != nullptr) _container[0]->query_region(region, subset, zoom + 1);
         if (_container[1] != nullptr) _container[1]->query_region(region, subset, zoom + 1);
         if (_container[2] != nullptr) _container[2]->query_region(region, subset, zoom + 1);
         if (_container[3] != nullptr) _container[3]->query_region(region, subset, zoom + 1);
      }
   }
}

void SpatialElement::aggregate_tile(uint64_t resolution, binned_container& subset, uint64_t zoom) const {
   const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

   if (zoom == resolution || value.leaf) {
      subset.emplace_back(&el);
   } else {
      if (_container[0] != nullptr) _container[0]->aggregate_tile(resolution, subset, zoom + 1);
      if (_container[1] != nullptr) _container[1]->aggregate_tile(resolution, subset, zoom + 1);
      if (_container[2] != nullptr) _container[2]->aggregate_tile(resolution, subset, zoom + 1);
      if (_container[3] != nullptr) _container[3]->aggregate_tile(resolution, subset, zoom + 1);
   }
}
