#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

void Dimension::restrict(range_container& range, range_container& response, binned_container& subset, CopyOption& option) {
   
   if (subset.size() == 0) return;

   // sort range only when necessary
   swap_and_sort(range, response);

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

   subset.clear();

   if (option == CopyValueFromSubset) option = CopyValueFromRange;
}

std::string Dimension::serialize(const Query& query, range_container& range, range_container& response, binned_container& subset, CopyOption option) {

   range.swap(response);

   // sort range only when necessary
   std::sort(range.begin(), range.end());

   if (range.size() == 0) return std::string();

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   // start json
   writer.StartArray();
   
   switch (query.type()) {
      case Query::TILE: 
         write_map<spatial_t, std::unordered_map>(writer, range, subset, option);
         break;
      case Query::GROUP: 
         write_map<categorical_t, std::map>(writer, range, subset, option);
         break;
      case Query::TSERIES: 
         write_map<temporal_t, std::map>(writer, range, subset, option);
         break;
      case Query::SCATTER: break;
      case Query::MYSQL: break;
      case Query::REGION: 
         write_count(writer, range, subset);
         break;
      default: break;
   }

   // end json
   writer.EndArray();
   return buffer.GetString();
}

void Dimension::write_count(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, binned_container& subset) {
   pivot_iterator it_lower;
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

   writer.StartArray();
   writer.Uint(count);
   writer.EndArray();
}
