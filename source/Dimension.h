#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class NDS;

class Dimension {
public:
   enum Type { Spatial, Temporal, Categorical };

   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
   virtual ~Dimension() = default;

   virtual bool query(const Query& query, subset_container& subsets) const = 0;
   virtual uint32_t build(const build_ctn& range, build_ctn& response, const link_ctn& links, link_ctn& share, NDS& nds) = 0;

   static std::string serialize(const Query& query, subset_container& subsets, const BinnedPivot& root);

protected:
   static void restrict(range_container& range, range_container& response, const subset_t& subset, CopyOption& option);
   static void restrictSubsetCopy(range_container& range, range_container& response, const subset_t& subset);
   static void restrictRangeCopy(range_container& range, range_container& response, const subset_t& subset);
   static void restrictDefaultCopy(range_container& range, range_container& response, const subset_t& subset);

   template<typename T>
   static void write_subset(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset);

   template<typename T, template<typename...> typename Container>
   static void write_range(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset);

   static void write_count(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset);

   static void write_pivtos(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset);

   static inline bool search_iterators(range_iterator& it_range, const range_container& range,
                                       pivot_it& it_lower, pivot_it& it_upper, const pivot_ctn& subset) {
      if (it_lower == subset.end() || it_range == range.end()) return false;

      if ((*it_range).pivot.ends_before(*it_lower)) {
         // binary search to find range iterator
         it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);

         if (it_range == range.end()) return false;
      }

      if ((*it_range).pivot.begins_after(*it_lower)) {
         if (it_lower == std::prev(subset.end())) return false;

         // binnary search to find lower subset iterator
         it_lower = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);

         if (it_lower == subset.end()) return false;
      }

      it_upper = std::upper_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::upper_bound_comp);

      return true;
   }

   static inline void compactation(range_container& input, range_container& output, CopyOption option) {
      output.emplace_back(input.front());

      if (option == DefaultCopy || option == CopyValueFromSubset) {
         for (size_t i = 1; i < input.size(); ++i) {
            if (output.back().pivot.back() == input[i].pivot.front()) {
               output.back().pivot.back(input[i].pivot.back());
            } else {
               output.emplace_back(input[i]);
            }
         }
      } else {
         // TODO create benchmark to test /else/ statement
         for (size_t i = 1; i < input.size(); ++i) {
            if (output.back().pivot.back() == input[i].pivot.front() && input[i].value == output.back().value) {
               output.back().pivot.back(input[i].pivot.back());
            } else {
               output.emplace_back(input[i]);
            }
         }
      }
   }

   static inline void swap_and_sort(range_container& range, range_container& response, CopyOption option) {
      /*According to benchmark, this is a bit slower than std::sort() on randomized sequences, 
      but much faster on partially - sorted sequences.*/
      gfx::timsort(response.begin(), response.end());

      range.clear();

      // compaction (and swap) phase
      compactation(response, range, option);

      response.clear();
   }

   static inline uint32_t count_and_increment(pivot_it& it_lower, pivot_it& it_upper) {
      uint32_t count = 0;
      while (it_lower != it_upper) {
         count += (*it_lower++).size();
      }
      return count;
   }

   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};

template<typename T>
void Dimension::write_subset(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset) {
   std::vector<uint32_t> map(subset.size(), 0);

   for (auto el = 0; el < subset.size(); ++el) {
      pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
      range_iterator it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
         map[el] += count_and_increment(it_lower, it_upper);
         ++it_range;
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
void Dimension::write_range(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_ctn& subset) {
   Container<uint64_t, uint32_t> map;

   for (const auto& el : subset) {
      pivot_it it_lower = el->ptr().begin(), it_upper;
      range_iterator it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
         map[(*it_range).value] += count_and_increment(it_lower, it_upper);
         ++it_range;
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
