#include "stdafx.h"
#include "Temporal.h"

Temporal::Temporal(const std::string& key, const uint32_t bin, const uint8_t offset)
   : Dimension(key, bin, offset) { }

uint32_t Temporal::build(const building_container& range, building_container& response, Data& data) {
   uint32_t pivots_count = 0;

   std::map<temporal_t, std::vector<Pivot>> tmp_container;

   for (const auto& ptr : range) {
      std::map<temporal_t, temporal_t> used;

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
         temporal_t value = data.record<temporal_t>(i, _offset);
         value = (temporal_t)(std::floor(value / (float)_bin) * (float)_bin);
         
         data.setHash(i, value);
         used[value]++;
      }

      uint32_t accum = ptr.front();
      for (const auto& entry : used) {

         uint32_t first = accum;
         accum += entry.second;
         uint32_t second = accum;

         tmp_container[entry.first].emplace_back(first, second);

         response.emplace_back(first, second);
         pivots_count++;
      }

      data.sort(ptr.front(), ptr.back());
   }

   _container.assign(tmp_container.begin(), tmp_container.end());
   _container.shrink_to_fit();

   return pivots_count;
}

bool Temporal::query(const Query& query, const response_container& range, response_container& response) const {

   const auto interval_it = query.interval().find(_key);

   if (interval_it == query.interval().end()) return false;

   auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), (*interval_it).second.bound[0]);
   auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), (*interval_it).second.bound[1]);

   for (const auto& r : range) {
      for(auto date_it = it_lower_data; date_it != it_upper_date; ++date_it) {

         const auto& subset = (*date_it).container;

         auto it_lower = std::lower_bound(subset.begin(), subset.end(), r.pivot, Pivot::lower_bound_comp);
         auto it_upper = std::upper_bound(it_lower, subset.end(), r.pivot, Pivot::upper_bound_comp);

         // case 0
         response.insert(response.end(), it_lower, it_upper);

         // case 1
         /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, r.value); });*/

         // case 2
         /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, (*date_it).date); });*/

         if (r.pivot.endsWith(*(--it_upper))) break;
      }
   }

   return true;
}
