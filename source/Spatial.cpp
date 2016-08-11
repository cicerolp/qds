#include "stdafx.h"
#include "NDS.h"
#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   _container.set_range(range);
   pivots_count += _container.expand(data, response, _offset, _bin);

   std::sort(response.begin(), response.end());

   return pivots_count;
}

bool Spatial::query(const Query& query, range_container& range, range_container& response, binned_container& subset, binned_container& subset_exp, CopyOption& option) const {

   if (!query.eval(_key)) return true;

   auto restriction = query.get<Query::spatial_query_t>(_key);

   if (restriction->tile.size()) {
      _container.query_tile(restriction->tile[0], restriction->resolution, subset_exp, 0);
   } else if (restriction->region.size()) {
      _container.query_region(restriction->region[0], subset_exp, 0);
   }

   // restrict only when necessary
   auto result = restrict(range, response, subset, subset_exp, option);

   if (query.type() == Query::TILE) option = CopyValueFromSubset;

   return result;
}
