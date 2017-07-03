#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
 public:
  Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
  ~Spatial() = default;

  uint32_t build(const build_ctn& range, build_ctn& response,
                 const link_ctn& links, link_ctn& share, NDS& nds) override;
  bool query(const Query& query, subset_container& subsets) const override;

 private:
  std::unique_ptr<SpatialElement> _tree;
};
