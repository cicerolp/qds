#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
 public:
  Categorical(const DimensionSchema &schema);
  ~Categorical() = default;

  uint32_t build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) override;
  bool query(const Query &query, subset_ctn &subsets) const override;

 private:
  std::vector<categorical_t> parse(const std::string &str) const;

  stde::dynarray<bined_pivot_t> _container;
};
