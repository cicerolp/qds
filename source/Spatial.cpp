#include "stdafx.h"
#include "Spatial.h"

Spatial::Spatial(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   for (const auto& ptr : range) {
      _container.emplace_back(std::make_unique<SpatialElement>(ptr, spatial_t(0, 0, 0)));
      pivots_count += _container.back()->build(response, data, 0);      
   }
   _container.shrink_to_fit();

   return pivots_count;
}
