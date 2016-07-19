#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"

class SpatialElement {
public:
   SpatialElement(const spatial_t& tile);
   ~SpatialElement() = default;

   uint32_t build(const Pivot& pivot, Data& data, const uint8_t offset);
   void query(const Query& query, std::vector<const SpatialElement*>& subset) const;

   spatial_t value;
   std::vector<Pivot> pivots;

private:
   uint32_t expand(Data& data, const uint8_t offset);

   void aggregate_tile(const Query& query, std::vector<const SpatialElement*>& subset) const;

   static const uint32_t max_levels{ 25 };

   std::array<std::unique_ptr<SpatialElement>, 4> _container;   
};
