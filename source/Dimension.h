#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   enum Type {
      Spatial,
      Temporal,
      Categorical
   };

   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);

   virtual ~Dimension() = default;

   virtual bool query(const Query& query, subset_container& subsets) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

   static std::string serialize(const Query& query, subset_container& subsets, const BinnedPivot& root);

protected:
   /*struct k_way_merge {
      using range_pair = std::pair<range_iterator, range_iterator>;

      inline bool operator()(const range_pair& lhs, const range_pair& rhs) const {
         return (*lhs.first).pivot.front() > (*rhs.first).pivot.front();
      }

      static inline void sort(range_container &output, std::vector<range_container> &input) {
         std::priority_queue<range_pair, std::vector<range_pair>, k_way_merge> pq;

         for (size_t i = 0; i < input.size(); i++) {
            if (!input[i].empty())
               pq.emplace(input[i].begin(), input[i].end());
         }

         while (!pq.empty()) {
            auto it_pair = pq.top();
            pq.pop();

            output.emplace_back((*it_pair.first).pivot, (*it_pair.first).value);
            ++it_pair.first;

            if (it_pair.first != it_pair.second) pq.push(it_pair);
         }
      }
   };*/

   static void restrict(range_container& range, range_container& response, const subset_t& subset, CopyOption& option);

   template<typename T>
   static void write_subset(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset);

   template<typename T, template<typename...> typename Container>
   static void write_range(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset);

   static void write_count(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset);

   static void write_pivtos(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset);

   static inline bool search_iterators(range_iterator& it_range, const range_container& range,
                                       pivot_iterator& it_lower, const pivot_container& subset) {
      if (it_lower == subset.end()) {
         it_range = range.end();
         return false;
      }

      if ((*it_range).pivot.ends_before(*it_lower)) {
         // binary search to find range iterator
         it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);
         if (it_range == range.end()) return false;
      }

      if ((*it_range).pivot.begins_after(*it_lower)) {
         if (it_lower == std::prev(subset.end())) {
            it_range = range.end();
            return false;
         }
         // binnary search to find lower subset iterator
         pivot_iterator it = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);
         if (it == subset.end()) return false;
         it_lower = it;
      }

      return true;
   }

   static inline void swap_and_sort(range_container& range, range_container& response, CopyOption option) {
      //std::sort(response.begin(), response.end());      
      boost::sort::parallel::parallel_sort(response.begin(), response.end());

      // TODO create benchmark to test /else/ statement

      range.clear();
      range.emplace_back(response.front());

      if (option == DefaultCopy || option == CopyValueFromSubset) {
         for (size_t i = 1; i < response.size(); ++i) {
            if (response[i].pivot.front() == range.back().pivot.back()) {
               range.back().pivot.back(response[i].pivot.back());
            } else {
               range.emplace_back(response[i]);
            }
         }
      } else {
         for (size_t i = 1; i < response.size(); ++i) {
            if (response[i].pivot.front() == range.back().pivot.back() && response[i].value == range.back().value) {
               range.back().pivot.back(response[i].pivot.back());
            } else {
               range.emplace_back(response[i]);
            }
         }
      }

      /*if (option == DefaultCopy || option == CopyValueFromSubset) {
         range.clear();
         range.emplace_back(response.front());
         for (size_t i = 1; i < response.size(); ++i) {
            if (response[i].pivot.front() == range.back().pivot.back()) {
               range.back().pivot.back(response[i].pivot.back());
            } else {
               range.emplace_back(response[i]);
            }
         }
      } else {         
         range.swap(response);
      }*/

      response.clear();
   }

   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};

template<typename T>
void Dimension::write_subset(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset) {
   std::vector<uint32_t> map(subset.size(), 0);

   for (auto el = 0; el < subset.size(); ++el) {
      auto it_lower = subset[el]->pivots.begin();
      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
         if (!search_iterators(it_range, range, it_lower, subset[el]->pivots)) break;
         while (it_lower != subset[el]->pivots.end() && (*it_range).pivot >= (*it_lower)) {
            map[el] += (*it_lower++).size();
         }
      }

      if (map[el] == 0) continue;

      // serialization
      writer.StartArray();
      toJSON(writer, (*(T*)&subset[el]->value));
      writer.Uint(map[el]);
      writer.EndArray();
   }
}

template<typename T, template <typename ...> class Container>
void Dimension::write_range(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset) {
   Container<uint64_t, uint32_t> map;

   for (const auto& el : subset) {
      auto it_lower = el->pivots.begin();
      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
         if (!search_iterators(it_range, range, it_lower, el->pivots)) break;
         while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
            map[(*it_range).value] += (*it_lower++).size();
         }
      }
   }

   // serialization
   for (const auto& pair : map) {
      writer.StartArray();
      toJSON(writer, (*(T*)&pair.first));
      writer.Uint(pair.second);
      writer.EndArray();
   }
}
