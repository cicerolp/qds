#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"

class SpatialElement {
   static const uint32_t max_levels{ 8 };

public:
   SpatialElement(const spatial_t& tile);
   SpatialElement(const spatial_t& tile, const building_container& container);
   ~SpatialElement() = default;

   inline void set_range(const building_container& range);

   uint32_t expand(Data& data, building_container& response, uint32_t offset, uint32_t bin);

   void query_tile(const spatial_t& tile, uint64_t resolution, binned_container& subset, uint64_t zoom) const;
   void query_region(const region_t& region, binned_container& subset, uint64_t zoom) const;

   binned_t el;

private:
   void aggregate_tile(uint64_t resolution, binned_container& subset, uint64_t zoom) const;
   inline bool count_expand(uint32_t bin) const;

   std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

void SpatialElement::set_range(const building_container& range) {
   el.pivots = pivot_container(range.size());
   std::memcpy(&el.pivots[0], &range[0], range.size() * sizeof(Pivot));
}

bool SpatialElement::count_expand(uint32_t bin) const {
   uint32_t count = 0;
   for (auto& ptr : el.pivots) {
      count += ptr.size();
   }
   return (count > bin) ? true : false;
}