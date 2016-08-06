#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   enum CopyOption {
      CopyValueFromRange,
      CopyValueFromSubset,
      DefaultCopy
   };

   enum Type {
      Spatial,
      Temporal,
      Categorical
   };

   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
      : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)) { }

   virtual ~Dimension() = default;

   virtual void query(const Query& query, range_container& range, range_container& response, binned_container& subset, CopyOption& option) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

   
   static std::string serialize(const Query& query, range_container& range, range_container& response, binned_container& subset, CopyOption option);

protected:
   
   static void restrict(range_container& range, range_container& response, binned_container& subset, CopyOption& option);

   template<typename T, template<typename...> typename Container>
   static void write_map(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, binned_container& subset, CopyOption option);

   static void write_count(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, binned_container& subset);

   static inline bool search_iterators(range_iterator& it_range, const range_container& range,
                                       pivot_iterator& it_lower, const pivot_container& subset) {
      if (it_lower == subset.end()) return false;

      if ((*it_range).pivot.ends_before(*it_lower)) {
         // binary search to find range iterator
         it_range = std::lower_bound(it_range, range.end(), (*it_lower), BinnedPivot::upper_bound_comp);
         if (it_range == range.end()) return false;
      }

      if ((*it_range).pivot.begins_after(*it_lower)) {
         // binnary search to find lower subset iterator
         pivot_iterator it = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);
         if (it == subset.end()) return false;
         it_lower = it;
      }

      return true;
   }

   static inline void swap_and_sort(range_container& range, range_container& response) {
      range.swap(response);
      std::sort(range.begin(), range.end());
      response.clear();
   }

   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};

template<typename T, template <typename ...> class Container>
void Dimension::write_map(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, binned_container& subset, CopyOption option) {

   pivot_iterator it_lower;
   Container<uint64_t, uint32_t> map;

   for (const auto& el : subset) {
      it_lower = el->pivots.begin();
      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
         if (!search_iterators(it_range, range, it_lower, el->pivots)) break;

         if (option == CopyValueFromRange) {
            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               map[(*it_range).value] += (*it_lower++).size();
            }
         } else if (option == CopyValueFromSubset) {
            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               map[el->value] += (*it_lower++).size();
            }
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
