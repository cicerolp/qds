#include "Spatial.h"

Spatial::Spatial(const DimensionSchema &schema)
    : Dimension(schema) {}

uint32_t Spatial::build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) {
  data.prepareOffset<coordinates_t>(_schema.offset);

  _tree = std::make_unique<SpatialElement>(nds, data, range, links, spatial_t(0, 0, 0));

  uint32_t pivots_count = _tree->expand(nds, data, range, links, _schema);

  return pivots_count;
}

bool Spatial::query(const Query &query, subset_ctn &subsets) const {
  auto clausule = query.get_const(_schema.index);

  if (clausule != nullptr) {
    subset_t subset;

    if (clausule->first == "tile") {
      auto tile = parse_tile(clausule->second);

      if (query.group_by(_schema.index)) {
        subset.option = CopyValueFromSubset;
      }

      _tree->query_tile(tile.tile, tile.resolution, subset.container);

    } else if (clausule->first == "region") {
      auto region = parse_region(clausule->second);

      if (query.group_by(_schema.index)) {
        subset.option = CopyValueFromSubset;
      }

      _tree->query_region(region, subset.container);
    }

    if (!subset.container.empty()) {
      subsets.emplace_back(subset);
      return true;
    } else {
      return false;
    }
  }

  return true;
}
tile_t Spatial::parse_tile(const std::string &str) {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  std::vector<std::string> tokens;
  boost::split(tokens, clausule, boost::is_any_of(":"));

  // [x]:[y]:[z]:[resolution]
  return tile_t(std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]));
}

region_t Spatial::parse_region(const std::string &str) {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  std::vector<std::string> tokens;
  boost::split(tokens, clausule, boost::is_any_of(":"));

  // [x0]:[y0]:[x1]:[y1]:[z]
  return region_t(std::stoi(tokens[0]),
                  std::stoi(tokens[1]),
                  std::stoi(tokens[2]),
                  std::stoi(tokens[3]),
                  std::stoi(tokens[4]));
}

void Spatial::get_schema_hint(rapidjson::Writer<rapidjson::StringBuffer> &writer) const {
  writer.Key("hint");
  writer.Int(g_Quadtree_Depth);
}
