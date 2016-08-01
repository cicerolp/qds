#include "stdafx.h"
#include "NDS.h"
#include "Categorical.h"

Categorical::Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(_bin) { }

uint32_t Categorical::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   std::vector<building_container> tmp_container(_bin);

   for (const auto& ptr : range) {
      std::vector<uint32_t> used(_bin, 0);

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
         categorical_t value = data.record<categorical_t>(i, _offset);

         data.setHash(i, value);
         used[value]++;
      }

      uint32_t accum = ptr.front();
      for (uint32_t i = 0; i < _bin; ++i) {

         if (used[i] == 0) continue;

         uint32_t first = accum;
         accum += used[i];
         uint32_t second = accum;

         tmp_container[i].emplace_back(first, second);

         response.emplace_back(first, second);
         pivots_count++;
      }

      data.sort(ptr.front(), ptr.back());
   }

   for (uint32_t index = 0; index < _bin; ++index) {
      _container[index].value = index;
      _container[index].pivots = pivot_container(tmp_container[index].size());
      std::memcpy(&_container[index].pivots[0], &tmp_container[index][0], tmp_container[index].size() * sizeof(Pivot));
   }

   return pivots_count;
}

bool Categorical::query(const Query& query, range_container& range, response_container& response, binned_container& subset, bool& pass_over_target) const {

   if (query.eval_where(_key)) {
      const auto value_it = query.where(_key);
      const bool target = query.eval_field(_key);

      if (value_it.size() == _bin && !target) return false;

      for (const auto& value : value_it) {
         subset.emplace_back(&_container[value]);
      }

      if (target) {
         pass_over_target = true;
         restrict(range, response, subset, CopyValueFromSubset);

      } else if (pass_over_target) {
         restrict(range, response, subset, CopyValueFromRange);

      } else {
         restrict(range, response, subset, DefaultCopy);
      }

   } else if (query.eval_field(_key)) {
      for (const auto& el : _container) {
         subset.emplace_back(&el);
      }

      pass_over_target = true;
      restrict(range, response, subset, CopyValueFromSubset);      

   } else {
      return false;
   }
   return true;
}
