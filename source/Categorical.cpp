#include "stdafx.h"
#include "NDS.h"
#include "Categorical.h"

Categorical::Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple) {
   for (uint32_t i = 0; i < _bin; ++i) {
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

   for (auto& el : _container) el.container.shrink_to_fit();
   _container.shrink_to_fit();

   return pivots_count;
}

bool Categorical::query(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const {

   if (query.eval_where(_key)) {
      return query_where(query, range, response, pass_over_target);

   } else if (query.eval_field(_key)) {
      return query_field(query, range, response, pass_over_target);

   } else {
      return false;
   }
}

bool Categorical::query_where(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const {
   const auto value_it = query.where(_key);
   const bool target = query.eval_field(_key);

   if (value_it.size() == _bin && !target) return false;

   // sort range only when necessary
   NDS::swap_and_sort(range, response);

   for (const auto& value : value_it) {

      auto iters_it = _container[value].container.begin();
      const auto& subset = _container[value].container;

      for (const auto& r : range) {

         if (iters_it == subset.end()) break;
         else if (r.pivot.begins_after(*iters_it)) break;
         else if (r.pivot.ends_before(*iters_it)) continue;

         building_iterator it_lower = iters_it;
         if (!(r.pivot >= (*it_lower))) {
            it_lower = std::lower_bound(iters_it, subset.end(), r.pivot, Pivot::lower_bound_comp);
            if (it_lower == subset.end()) continue;
         }

         while (it_lower != subset.end() && r.pivot >= (*it_lower)) {
            if (target) {
               // case 2
               response.emplace_back((*it_lower), value);
            } else if (pass_over_target) {
               // case 1
               response.emplace_back((*it_lower), r.value);
            } else {
               // case 0
               response.emplace_back((*it_lower));
            }
            it_lower++;
         }

         iters_it = it_lower;
      }
   }

   if (target) {
      pass_over_target = true;
   }

   return true;
}

bool Categorical::query_field(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const {

   // sort range only when necessary
   NDS::swap_and_sort(range, response);

   for (const auto& el : _container) {

      auto iters_it = el.container.begin();
      const auto& subset = el.container;

      for (const auto& r : range) {

         if (iters_it == subset.end()) break;
         else if (r.pivot.begins_after(*iters_it)) break;
         else if (r.pivot.ends_before(*iters_it)) continue;

         building_iterator it_lower = iters_it;
         if (!(r.pivot >= (*it_lower))) {
            it_lower = std::lower_bound(iters_it, subset.end(), r.pivot, Pivot::lower_bound_comp);
            if (it_lower == subset.end()) continue;
         }

         while (it_lower != subset.end() && r.pivot >= (*it_lower)) {
            // case 2
            response.emplace_back((*it_lower), el.value);

            it_lower++;
         }

         iters_it = it_lower;
      }
   }

   pass_over_target = true;
   return true;
}
