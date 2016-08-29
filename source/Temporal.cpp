#include "stdafx.h"
#include "NDS.h"
#include "Temporal.h"

Temporal::Temporal(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple) { }

uint32_t Temporal::build(const build_ctn& range, build_ctn& response, const link_ctn& links, link_ctn& share, NDS& nds) {
   nds.data()->prepareOffset<temporal_t>(_offset);

   uint32_t pivots_count = 0;

   std::map<temporal_t, std::vector<Pivot>> tmp_ctn;

   for (const auto& ptr : range) {
      std::map<temporal_t, uint32_t> used;

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
         temporal_t value = nds.data()->record<temporal_t>(i);
         value = (temporal_t)(std::floor(value / (float)_bin) * (float)_bin);

         nds.data()->setHash(i, value);
         ++used[value];
      }

      uint32_t accum = ptr.front();
      for (const auto& entry : used) {

         uint32_t first = accum;
         accum += entry.second;
         uint32_t second = accum;

         tmp_ctn[entry.first].emplace_back(first, second);

         response.emplace_back(first, second);
         pivots_count++;
      }

      nds.data()->sort(ptr.front(), ptr.back());
   }

   _container = stde::dynarray<TemporalElement>(tmp_ctn.size());

   uint32_t index = 0;
   for (auto& pair : tmp_ctn) {
      _container[index].el.value = pair.first;
      nds.share(_container[index].el, pair.second, links, share);
      ++index;
   }

   nds.data()->dispose();

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
