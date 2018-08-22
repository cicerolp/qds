#pragma once

#include "Data.h"
#include "NDS.h"
#include "Pivot.h"

extern uint32_t g_Quadtree_Depth;

class SpatialElement {
 public:
  SpatialElement(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links, const spatial_t &tile);
  SpatialElement(NDS &nds, Data &data, const build_ctn &container, const link_ctn &links, const spatial_t &tile);
  ~SpatialElement() = default;

  uint32_t expand(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links, const DimensionSchema &schema);

  void query_tile(const spatial_t &tile, uint64_t resolution, bined_ctn &subset) const;
  void query_region(const region_t &region, bined_ctn &subset) const;

  inline link_ctn get_link() const {
    return {_el.pivots};
  }

 private:
  void aggregate_tile(uint64_t resolution, bined_ctn &subset) const;
  inline bool count_expand(uint32_t bin) const;

  inline static spatial_t get_tile(uint32_t x, uint32_t y, uint32_t z, uint32_t index) {
    if (index == 1) {
      ++y;
    } else if (index == 2) {
      ++x;
    } else if (index == 3) {
      ++y;
      ++x;
    }
    return spatial_t(x, y, z);
  }

  bined_pivot_t _el;
  std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

bool SpatialElement::count_expand(uint32_t bin) const {
#ifdef NDS_OPTIMIZE_LEAF
  if (_el.ptr().size() > bin) return true;
#else
  if (_el.ptr().size() >= bin) return true;
#endif

  uint32_t count = 0;
  for (auto &ptr : _el.ptr()) {
    count += ptr.size();
  }

#ifdef NDS_OPTIMIZE_LEAF
  return (count > bin) ? true : false;
#else
  return (count >= bin) ? true : false;
#endif
}
