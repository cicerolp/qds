#include "stdafx.h"
#include "NDS.h"
#include "Temporal.h"

Temporal::Temporal(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple) { }

uint32_t Temporal::build(const building_container& range, building_container& response, Data& data) {
   
   data.prepareOffset<temporal_t>(_offset);
   
   uint32_t pivots_count = 0;

   std::map<temporal_t, std::vector<Pivot>> tmp_container;

   for (const auto& ptr : range) {
      std::unordered_map<temporal_t, uint32_t> used;

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
         temporal_t value = data.record<temporal_t>(i);
         value = (temporal_t)(std::floor(value / (float)_bin) * (float)_bin);

         data.setHash(i, value);
         ++used[value];
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

   _container = stde::dynarray<TemporalElement>(tmp_container.size());

   uint32_t index = 0;
   for (auto& pair : tmp_container) {
      _container[index].el.value = pair.first;
      _container[index].el.pivots = pivot_container(pair.second.size());
      std::memcpy(&_container[index].el.pivots[0], &pair.second[0], pair.second.size() * sizeof(Pivot));
      ++index;
   }

   data.dispose();

   return pivots_count;
}

bool Temporal::query(const Query& query, subset_container& subsets) const {

   if (!query.eval(_key)) return true;

   const auto& interval = query.get<Query::temporal_query_t>(_key)->interval;

   if (query.type() != Query::TSERIES && interval.contain(_container.front().el.value, _container.back().el.value)) {
      return true;
   }

   subset_t subset;

   auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), interval.bound[0]);
   auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), interval.bound[1]);
   
   for (auto it = it_lower_data; it < it_upper_date; ++it) {
      subset.container.emplace_back(&(*it).el);
   }

   if (query.type() == Query::TSERIES) subset.option = CopyValueFromSubset;

   if (subset.container.size() != 0) {
      subsets.emplace_back(subset);
      return true;
   } else return false;
}
