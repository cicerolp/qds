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

bool Temporal::query(const Query& query, response_container& range, response_container& response, bool& pass_over_target) const {

   const auto interval_it = query.interval().find(_key);

   if (interval_it == query.interval().end()) return false;

   if (query.type() != Query::TSERIES && (*interval_it).second.contain(_container.front().date, _container.back().date)) {
      return false;
   }

   auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), (*interval_it).second.bound[0]);
   auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), (*interval_it).second.bound[1]);

   std::unordered_map<const TemporalElement*, building_iterator> iters;
   for (auto date_it = it_lower_data; date_it != it_upper_date; ++date_it) {
      iters.emplace(&(*date_it), (*date_it).container.begin());
   }

   // sort range only when necessary
   std::sort(range.begin(), range.end());

   for (auto date_it = it_lower_data; date_it != it_upper_date; ++date_it) {

      auto& iters_it = iters.at(&(*date_it));
      const auto& subset = (*date_it).container;

      for (const auto& r : range) {

         if (iters_it == subset.end()) break;
         else if (!r.pivot.intersect_range((*iters_it), subset.back())) continue;

         building_iterator it_lower = std::lower_bound(iters_it, subset.end(), r.pivot, Pivot::lower_bound_comp);

         if (it_lower == subset.end()) continue;

         while (it_lower != subset.end() && r.pivot >= (*it_lower)) {
            if (pass_over_target) {
               // case 1
               response.emplace_back((*it_lower), r.value);
            } else if (query.type() == Query::TSERIES) {
               // case 2
               response.emplace_back((*it_lower), (*date_it).date);
            } else {
               // TODO optimize case 0
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
