#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"

class SpatialElement {
   static const uint32_t max_levels{ 25 };
   static const uint32_t leaf_size{ 32 };

public:
   SpatialElement(const spatial_t& tile);
   SpatialElement(const spatial_t& tile, const building_container& container);
   ~SpatialElement() = default;

   inline void set_range(const building_container& range);

   uint32_t expand(Data& data, building_container& response, const uint8_t offset);

   void query_tile(const std::vector<spatial_t>& tile, uint8_t resolution, binned_container& subset, uint8_t z) const;
   void query_region(const std::vector<region_t>& region, binned_container& subset, uint8_t z) const;

   binned_t el;

private:
   void aggregate_tile(const std::vector<spatial_t>& tile, uint8_t resolution, binned_container& subset, uint8_t z) const;
   inline bool count_expand() const;

   std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

void SpatialElement::set_range(const building_container& range) {
   el.pivots = pivot_container(range.size());
   std::memcpy(&el.pivots[0], &range[0], range.size() * sizeof(Pivot));
}

bool SpatialElement::count_expand() const {
   uint32_t count = 0;
   for (auto& ptr : el.pivots) {
      count += ptr.size();
   }
   return (count > leaf_size) ? true : false;
}