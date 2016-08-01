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

bool Spatial::query(const Query& query, range_container& range, response_container& response, binned_container& subset, bool& pass_over_target) const {

   if (query.eval_tile(_key)) {
      _container.query_tile(query, subset, 0);
   } else if (query.eval_region(_key)) {
      _container.query_region(query, subset, 0);
   } else {
      return false;
   }

   if (pass_over_target) {
      restrict(range, response, subset, CopyValueFromRange);
   } else if (query.type() == Query::TILE) {
      pass_over_target = true;
      restrict(range, response, subset, CopyValueFromSubset);
   } else {
      restrict(range, response, subset, DefaultCopy);
   }

   return true;
}
