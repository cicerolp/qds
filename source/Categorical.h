#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
 public:
  Categorical(const DimensionSchema &schema);
  ~Categorical() = default;

  uint32_t build(const build_ctn &range,
                 build_ctn &response,
                 const link_ctn &links,
                 link_ctn &share,
                 NDS &nds) override;
  bool query(const Query &query, subset_ctn &subsets) const override;

 private:
  std::vector<categorical_t> parse(const std::string &str) const;

  stde::dynarray<bined_pivot_t> _container;
};
