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

   if (query.where().find(_key) == query.where().end()) {
      return false;
   }

   for (const auto& r : range) {
      for (const auto& value : query.where()[_key]) {

         /*const auto it = std::lower_bound(_container[value].container.begin(), _container[value].container.end(), r.pivot,
                                          [](const Pivot& e1, const Pivot& e2) {
                                             return e1.front() < e2.front();
                                          });*/

         const auto pivot_it = std::lower_bound(_container[value].container.begin(), _container[value].container.end(), r.pivot);

         if (pivot_it != _container[value].container.end() && r.pivot.contains(*pivot_it)) {
            response.emplace_back((*pivot_it), value);

            if (r.pivot.endsWith(*pivot_it)) break;
         }
      }
   }

   return true;
}
