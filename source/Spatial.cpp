#include "stdafx.h"
#include "NDS.h"
#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   _container.set_range(range);
   pivots_count += _container.expand(data, response, _offset);

   std::sort(response.begin(), response.end());

   return pivots_count;
}

std::string Spatial::query(const Query& query, range_container& range, response_container& response, CopyOption& option) const {

   if (!query.eval(_key)) return std::string();

   binned_container subset;
   auto restriction = query.get<Query::spatial_query_t>(_key);

   if (restriction->tile.size()) {
      _container.query_tile(restriction->tile, restriction->resolution, subset, 0);
   } else if (query.get<Query::spatial_query_t>(_key)->region.size()) {
      _container.query_region(restriction->region, subset, 0);
   } else {
      return std::string();
   }

   if (query.type() == Query::TILE) {
      //restrict(range, response, subset, CopyValueFromSubset);
      //option = CopyValueFromRange;

      return serialize(query, response, subset, CopyValueFromSubset);

   } else {
      //restrict(range, response, subset, option);

      return serialize(query, response, subset, option);
   }

   return std::string();
}
