#include "stdafx.h"
#include "NDS.h"
#include "Categorical.h"

Categorical::Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(_bin) { }

uint32_t Categorical::build(const building_container& range, building_container& response, NDS& nds) {
   nds.data()->prepareOffset<categorical_t>(_offset);

   uint32_t pivots_count = 0;

   std::vector<building_container> tmp_container(_bin);

   for (const auto& ptr : range) {
      std::vector<uint32_t> used(_bin, 0);

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
         categorical_t value = nds.data()->record<categorical_t>(i);

         nds.data()->setHash(i, value);
         ++used[value];
      }

      uint32_t accum = ptr.front();
      for (uint32_t i = 0; i < _bin; ++i) {

         if (used[i] == 0) continue;

         uint32_t first = accum;
         accum += used[i];
         uint32_t second = accum;

         tmp_container[i].emplace_back(first, second);

         response.emplace_back(first, second);
         ++pivots_count;
      }

      nds.data()->sort(ptr.front(), ptr.back());
   }

   for (uint32_t index = 0; index < _bin; ++index) {
      _container[index].value = index;
      nds.share(_container[index], tmp_container[index]);
   }

   nds.data()->dispose();

   return pivots_count;
}

bool Categorical::query(const Query& query, subset_container& subsets) const {

   if (!query.eval(_key)) return true;

   subset_t subset;
   const auto& restriction = query.get<Query::categorical_query_t>(_key);

   if (restriction->where.size()) {
      if (restriction->where.size() == _bin && !restriction->field) return true;

      for (const auto& value : restriction->where) {
         subset.container.emplace_back(&_container[value]);
      }

      if (restriction->field) subset.option = CopyValueFromSubset;

   } else if (restriction->field) {
      for (const auto& el : _container) {
         subset.container.emplace_back(&el);
      }

      subset.option = CopyValueFromSubset;
   }

   if (subset.container.size() != 0) {
      subsets.emplace_back(subset);
      return true;
   } else return false;
}
