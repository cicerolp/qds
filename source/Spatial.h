#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
 public:
  Spatial(const DimensionSchema &schema);
  ~Spatial() = default;

  uint32_t build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) override;
  bool query(const Query &query, subset_ctn &subsets) const override;

  void get_schema_hint(rapidjson::Writer<rapidjson::StringBuffer> &writer) const override;

  static tile_t parse_tile(const std::string &str);
  static region_t parse_region(const std::string &str);

  std::unique_ptr<SpatialElement> _tree;
};
