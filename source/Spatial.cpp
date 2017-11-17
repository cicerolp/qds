#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
    : Dimension(tuple) {}

uint32_t Spatial::build(const build_ctn& range, build_ctn& response,
                        const link_ctn& links, link_ctn& share, NDS& nds) {
  nds.data()->prepareOffset<coordinates_t>(_offset);

  _tree =
      std::make_unique<SpatialElement>(spatial_t(0, 0, 0), range, links, nds);

  uint32_t pivots_count = _tree->expand(response, _bin, share, nds);

  std::sort(response.begin(), response.end());

  nds.data()->dispose();

  return pivots_count;
}

bool Spatial::query(const Query& query, subset_container& subsets) const {
  const auto &restriction = query.eval<Query::spatial_restriction_t>(_key);

  if (!restriction || _tree == nullptr) return true;

  subset_t subset;

  //TODO fix resolution
  if (restriction->tile != nullptr) {
    _tree->query_tile((*restriction->tile), 8, subset.container, 0);

  } else if (restriction->region != nullptr) {
    _tree->query_region((*restriction->region), subset.container, 0);
  }

  if (query.aggregation() == Query::TILE) subset.option = CopyValueFromSubset;

  if (subset.container.size() != 0) {
    subsets.emplace_back(subset);
    return true;
  } else
    return false;
}
