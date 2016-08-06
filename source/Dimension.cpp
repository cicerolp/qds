#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

void Dimension::restrict(range_container& range, response_container& response, const binned_container& subset, CopyOption option) {
   // sort range only when necessary
   swap_and_sort(range, response);

   if (range.size() == 0) return;

   pivot_iterator it_lower;

   for (const auto& el : subset) {

      it_lower = el->pivots.begin();

      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {

         if (!search_iterators(it_range, range, it_lower, el->pivots)) break;
         
         switch (option) {
         case CopyValueFromRange:
            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               response.emplace_back((*it_lower++), (*it_range).value);
            }
            break;
         case CopyValueFromSubset:
            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               response.emplace_back((*it_lower++), el->value);
            }
            break;
         default:
            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               response.emplace_back((*it_lower++));
            }
            break;
         }
      }
   }
}

std::string Dimension::serialize(const Query& query, range_container& range, binned_container& subset, CopyOption option) {

   // sort range only when necessary
   std::sort(range.begin(), range.end());

   if (range.size() == 0) return std::string();

   pivot_iterator it_lower;

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   // start json
   writer.StartArray();

   if (query.type() == Query::TILE) {
      std::unordered_map<uint64_t, uint32_t> map;

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
         writer.Uint((*(spatial_t*)&pair.first).x);
         writer.Uint((*(spatial_t*)&pair.first).y);
         writer.Uint((*(spatial_t*)&pair.first).z);
         writer.Uint(pair.second);
         writer.EndArray();
      }
   } else if (query.type() == Query::REGION) {
      uint32_t count = 0;

      for (const auto& el : subset) {
         it_lower = el->pivots.begin();
         for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
            if (!search_iterators(it_range, range, it_lower, el->pivots)) break;

            while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
               count += (*it_lower++).size();
            }

         }
      }

      // serialization
      writer.StartArray();
      writer.Uint(count);
      writer.EndArray();
   }

   // end json
   writer.EndArray();
   return buffer.GetString();
}
