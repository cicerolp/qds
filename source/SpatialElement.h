#pragma once

#include "Data.h"
#include "NDS.h"
#include "Pivot.h"

extern uint32_t g_Quadtree_Depth;

class SpatialElement {
 public:
  SpatialElement(const spatial_t &tile, const build_ctn &container, const link_ctn &links, link_ctn &share, NDS &nds);
  SpatialElement(const spatial_t &tile, const build_ctn &container, const link_ctn &links, NDS &nds);
  ~SpatialElement() = default;

  uint32_t expand(build_ctn &response, uint32_t bin, link_ctn &share, NDS &nds);

  void query_tile(const spatial_t &tile, uint64_t resolution, subset_pivot_ctn &subset, uint64_t zoom) const;
  void query_region(const region_t &region, subset_pivot_ctn &subset, uint64_t zoom) const;

  inline link_ctn get_link() const {
    return {_el.pivots};
  }

 private:
  void aggregate_tile(uint64_t resolution, subset_pivot_ctn &subset, uint64_t zoom) const;
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
    return {x, y};
  }

  bined_pivot_t _el;
  std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

bool SpatialElement::count_expand(uint32_t bin) const {
#ifdef OPTIMIZE_LEAF
  if (_el.ptr().size() > bin) return true;
#else
  if (_el.ptr().size() >= bin) return true;
#endif

  uint32_t count = 0;
  for (auto &ptr : _el.ptr()) {
    count += ptr.size();
  }

#ifdef OPTIMIZE_LEAF
  return (count > bin) ? true : false;
#else
  return (count >= bin) ? true : false;
#endif
}
