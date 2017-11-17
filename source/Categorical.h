#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
 public:
  Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
  ~Categorical() = default;

  uint32_t build(const build_ctn& range, build_ctn& response,
                 const link_ctn& links, link_ctn& share, NDS& nds) override;
  bool query(const Query& query, subset_ctn& subsets) const override;

 private:
  stde::dynarray<bined_pivot_t> _container;
};
