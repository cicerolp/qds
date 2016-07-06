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
