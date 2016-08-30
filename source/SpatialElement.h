#pragma once

#include "NDS.h"
#include "Data.h"
#include "Pivot.h"

extern uint32_t g_Quadtree_Depth;

class SpatialElement {
public:
   SpatialElement(const spatial_t& tile, pivot_ctn* ptr);
   SpatialElement(const spatial_t& tile, const build_ctn& range, NDS& nds);
   SpatialElement(const spatial_t& tile, const build_ctn& range, const link_ctn& links, NDS& nds);
   ~SpatialElement() = default;

   uint32_t expand(build_ctn& response, uint32_t bin, link_ctn& share, NDS& nds);

   void query_tile(const spatial_t& tile, uint64_t resolution, binned_ctn& subset, uint64_t zoom) const;
   void query_region(const region_t& region, binned_ctn& subset, uint64_t zoom) const;

private:
   void aggregate_tile(uint64_t resolution, binned_ctn& subset, uint64_t zoom) const;
   inline bool count_expand(uint32_t bin) const;

   inline static std::pair<uint32_t, uint32_t> get_tile(uint32_t x, uint32_t y, uint32_t index) {
      if (index == 1) {
         ++y;
      } else if (index == 2) {
         ++x;
      } else if (index == 3) {
         ++y;
         ++x;
      }
      return { x, y };
   }

   binned_t el;
   std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

bool SpatialElement::count_expand(uint32_t bin) const {
   if (el.ptr().size() > bin) return true;

   uint32_t count = 0;
   for (auto& ptr : el.ptr()) {
      count += ptr.size();
   }
   return (count > bin) ? true : false;
}
