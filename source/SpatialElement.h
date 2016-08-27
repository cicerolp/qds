#pragma once

#include "NDS.h"
#include "Data.h"
#include "Pivot.h"

class SpatialElement {
   static const uint32_t max_levels{ 25 };

public:
   SpatialElement(const spatial_t& tile);
   ~SpatialElement() = default;

   inline void set_range(const building_container& range, NDS& nds);

   uint32_t expand(building_container& response, uint32_t bin, NDS& nds);

   void query_tile(const spatial_t& tile, uint64_t resolution, binned_container& subset, uint64_t zoom) const;
   void query_region(const region_t& region, binned_container& subset, uint64_t zoom) const;

private:
   void aggregate_tile(uint64_t resolution, binned_container& subset, uint64_t zoom) const;
   inline bool count_expand(uint32_t bin) const;

   binned_t el;
   std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

void SpatialElement::set_range(const building_container& range, NDS& nds) {
   nds.share(el, range);
}

bool SpatialElement::count_expand(uint32_t bin) const {
   if (el.ptr().size() > bin) return true;

   uint32_t count = 0;
   for (auto& ptr : el.ptr()) {
      count += ptr.size();
   }
   return (count > bin) ? true : false;
}