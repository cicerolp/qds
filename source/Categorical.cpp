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

bool Categorical::query(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const {

   if (query.where().find(_key) != query.where().end()) {
      return query_where(query, range, response, pass_over_target);
   } else if (query.field().find(_key) != query.field().end()) {
      return query_field(query, range, response, pass_over_target);
   } else {
      return false;
   }
}

bool Categorical::query_where(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const {
   const auto value_it = query.where().find(_key);
   const bool target = query.field().find(_key) != query.field().end();

   if ((*value_it).second.size() == _bin && !target) return false;

   std::unordered_map<const CategoricalElement*, building_iterator> iters;
   for (const auto& value : (*value_it).second) {
      iters.emplace(&_container[value], _container[value].container.begin());
   }

   for (const auto& r : range) {
      for (const auto& value : (*value_it).second) {

         const auto& subset = _container[value].container;

         // TODO assert optimization
         auto it_lower = std::lower_bound(iters[&_container[value]], subset.end(), r.pivot, Pivot::lower_bound_comp);
         //auto it_lower = std::lower_bound(subset.begin(), subset.end(), r.pivot, Pivot::lower_bound_comp);
         auto it_upper = std::upper_bound(it_lower, subset.end(), r.pivot, Pivot::upper_bound_comp);

         iters[&_container[value]] = it_lower;

         // case 0
         response.insert(response.end(), it_lower, it_upper);

         // case 2
         if (target) {
            /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, _container[value].value); });*/
            std::for_each(response.end() - (it_upper - it_lower), response.end(), [&](BinnedPivot& p) {
                             p.value = value;
                          });

            // case1
         } else if (pass_over_target) {
            /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, r.value); });*/
            std::for_each(response.end() - (it_upper - it_lower), response.end(), [&](BinnedPivot& p) {
                             p.value = r.value;
                          });
         }
      }
   }

   if (target) {
      pass_over_target = true;
   }

   return true;
}

bool Categorical::query_field(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const {

   std::unordered_map<const CategoricalElement*, building_iterator> iters;
   for (const auto& el : _container) {
      iters.emplace(&el, el.container.begin());
   }

   for (const auto& r : range) {
      for (const auto& el : _container) {

         const auto& subset = el.container;

         // TODO assert optimization
         auto it_lower = std::lower_bound(iters[&el], subset.end(), r.pivot, Pivot::lower_bound_comp);
         //auto it_lower = std::lower_bound(subset.begin(), subset.end(), r.pivot, Pivot::lower_bound_comp);
         auto it_upper = std::upper_bound(it_lower, subset.end(), r.pivot, Pivot::upper_bound_comp);

         iters[&el] = it_lower;

         // case 0
         response.insert(response.end(), it_lower, it_upper);

         // case 2
         /*std::transform(it_lower, it_upper, std::back_inserter(response), [&](const Pivot& p) { return BinnedPivot(p, _container[value].value); });*/
         std::for_each(response.end() - (it_upper - it_lower), response.end(), [&](BinnedPivot& p) {
                          p.value = el.value;
                       });
      }
   }

   pass_over_target = true;
   return true;
}
