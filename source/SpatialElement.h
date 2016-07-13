#pragma once

#include "Data.h"
#include "Pivot.h"

#include "mercator_util.h"

class SpatialElement {
public:
   SpatialElement(const spatial_t& tile);
   ~SpatialElement() = default;

   uint32_t build(const Pivot& range, building_container& response, Data& data, uint8_t zoom);

private:
   uint32_t expand(building_container& parent, const Pivot& pivot, building_container& response, Data& data, uint8_t zoom);

   const spatial_t _tile;

   std::vector<Pivot> _pivots;
   std::array<std::unique_ptr<SpatialElement>, 4> _container;   
};
