#pragma once

#include "Data.h"
#include "Pivot.h"

#include "mercator_util.h"

class SpatialElement {
public:
   SpatialElement(const Pivot& pivot, const spatial_t& tile);
   ~SpatialElement() = default;

   uint32_t build(building_container& response, Data& data, uint8_t zoom);

private:
   const Pivot _pivot;
   const spatial_t _tile;

   std::array<std::unique_ptr<SpatialElement>, 4> _container;   
};
