#include "stdafx.h"
#include "NDS.h"
#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple) { }

uint32_t Spatial::build(const build_ctn& range, build_ctn& response, const link_ctn& links, link_ctn& share, NDS& nds) {
   nds.data()->prepareOffset<coordinates_t>(_offset);

   _container = std::make_unique<SpatialElement>(spatial_t(0,0,0), range, links, nds);
   
   uint32_t pivots_count = 0;

   pivots_count += _container->expand(response, _bin, share, nds);

   std::sort(response.begin(), response.end());

   nds.data()->dispose();

   return pivots_count;
}

bool Spatial::query(const Query& query, subset_container& subsets) const {

   if (!query.eval(_key) || _container == nullptr) return true;

   subset_t subset;
   auto restriction = query.get<Query::spatial_query_t>(_key);

   if (restriction->tile.size()) {
      _container->query_tile(restriction->tile[0], restriction->resolution, subset.container, 0);
   } else if (restriction->region.size()) {
      _container->query_region(restriction->region[0], subset.container, 0);
   }

   if (query.type() == Query::TILE) subset.option = CopyValueFromSubset;

   if (subset.container.size() != 0) {
      subsets.emplace_back(subset);
      return true;
   } else return false;
}
