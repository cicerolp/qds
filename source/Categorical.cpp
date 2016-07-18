#include "stdafx.h"
#include "Categorical.h"

Categorical::Categorical(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset) {
   for (uint32_t i = 0; i < bin; ++i) {
      _container.emplace_back(i);
   }
}

uint32_t Categorical::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

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

         _container[i].container.emplace_back(first, second);

         response.emplace_back(first, second);
         pivots_count++;
      }

      data.sort(ptr.front(), ptr.back());
   }

   return pivots_count;
}

bool Categorical::query(const Query& query, const response_container& range, response_container& response) const {

   const auto value_it = query.where().find(_key);

   if (value_it == query.where().end()) return false;

   for (const auto& r : range) {
      for (const auto& value : (*value_it).second) {

         const auto& subset = _container[value].container;

         const auto it_lower = std::lower_bound(subset.begin(), subset.end(), r.pivot, Pivot::lower_bound_comp);
         const auto it_upper = std::upper_bound(it_lower, subset.end(), r.pivot, Pivot::upper_bound_comp);

         // case 0
         response.insert(response.end(), it_lower, it_upper);

         // case 1
         /*const uint64_t& rvalue = r.value;
         std::transform(it_lower, it_upper, std::back_inserter(response), [rvalue](const Pivot& p) { return BinnedPivot(p, rvalue); });*/

         // case 2
         /*const uint64_t& rvalue = _container[value].value;
         std::transform(it_lower, it_upper, std::back_inserter(response), [rvalue](const Pivot& p) { return BinnedPivot(p, rvalue); });*/

         // BUG remove comment
         //if (r.pivot.endsWith(*it)) break;
      }
   }

   return true;
}
