#include "stdafx.h"
#include "NDS.h"
#include "Temporal.h"

Temporal::Temporal(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple) { }

uint32_t Temporal::build(const building_container& range, building_container& response, Data& data) {
   uint32_t pivots_count = 0;

   std::map<temporal_t, std::vector<Pivot>> tmp_container;

   for (const auto& ptr : range) {
      std::map<temporal_t, uint32_t> used;

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
      
   _container = stde::dynarray<TemporalElement>(tmp_container.size());

   uint32_t index = 0;
   for (auto& pair : tmp_container) {
      _container[index].date = pair.first;
      _container[index].container = pivot_container(pair.second.size());
      std::memcpy(&_container[index].container[0], &pair.second[0], pair.second.size() * sizeof(Pivot));
      ++index;
   }

   return pivots_count;
}

bool Temporal::query(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const {

   if (!query.eval_interval(_key)) return false;

   const auto interval = query.interval(_key);

   if (query.type() != Query::TSERIES && interval.contain(_container.front().date, _container.back().date)) {
      return false;
   }

   auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), interval.bound[0]);
   auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), interval.bound[1]);

   // sort range only when necessary
   NDS::swap_and_sort(range, response);

   for (auto date_it = it_lower_data; date_it != it_upper_date; ++date_it) {

      auto iters_it = (*date_it).container.begin();
      const auto& subset = (*date_it).container;

      for (const auto& r : range) {

         if (iters_it == subset.end()) break;
         else if (r.pivot.begins_after(*iters_it)) break;
         else if (r.pivot.ends_before(*iters_it)) continue;

         pivot_iterator it_lower = iters_it;
         if (!(r.pivot >= (*it_lower))) {
            it_lower = std::lower_bound(iters_it, subset.end(), r.pivot, Pivot::lower_bound_comp);
            if (it_lower == subset.end()) continue;
         }

         while (it_lower != subset.end() && r.pivot >= (*it_lower)) {
            if (pass_over_target) {
               // case 1
               response.emplace_back((*it_lower), r.value);
            } else if (query.type() == Query::TSERIES) {
               // case 2
               response.emplace_back((*it_lower), (*date_it).date);
            } else {
               // case 0
               response.emplace_back((*it_lower));
            }
            it_lower++;
         }

         iters_it = it_lower;
      }
   }

   if (query.type() == Query::TSERIES) {
      pass_over_target = true;
   }

   return true;
}
