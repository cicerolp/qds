#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

Dimension::Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
                    : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)) {
   std::cout << "\t\tKey: [" << _key  << "], Bin: [" << _bin << "], Offset: [" << _offset << "]" << std::endl;
}

bool Dimension::restrict(range_container& range, range_container& response, binned_container& subset, binned_container& subset_exp, CopyOption& option) {
 
   if (subset_exp.size() != 0) {
      if (subset.size() == 0) {
         // initial query
         subset.swap(subset_exp);
         return true;
      }
   } else if (subset.size() != 0) {
      // empty result, nothing to compute
      subset.clear();
      return false;
   }

   // sort range only when necessary
   swap_and_sort(range, response, option);

   for (const auto& el : subset) {
      auto it_lower = el->pivots.begin();
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

   subset.swap(subset_exp);
   subset_exp.clear();

   if (option == CopyValueFromSubset) option = CopyValueFromRange;

   if (response.size() == 0) return false;
   else return true;
}

std::string Dimension::serialize(const Query& query, range_container& range, range_container& response, binned_container& subset, CopyOption option) {

   if (subset.size() == 0 || response.size() == 0) return std::string("[]");

   // sort range only when necessary
   swap_and_sort(range, response, option);

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   // start json
   writer.StartArray();

   switch (query.type()) {
      case Query::TILE: 
         if (option == CopyValueFromSubset) write_subset<spatial_t>(writer, range, subset);
         else write_range<spatial_t, std::unordered_map>(writer, range, subset);         
         break;
      case Query::GROUP: 
         if (option == CopyValueFromSubset) {
            std::sort(subset.begin(), subset.end(), binned_t::Comp);
            write_subset<categorical_t>(writer, range, subset);
         } else write_range<uint64_t, std::map>(writer, range, subset);
         break;
      case Query::TSERIES: 
         if (option == CopyValueFromSubset) {
            std::sort(subset.begin(), subset.end(), binned_t::Comp);
            write_subset<temporal_t>(writer, range, subset);
         } else write_range<uint64_t, std::map>(writer, range, subset);
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
   uint32_t count = 0;

   for (const auto& el : subset) {
      auto it_lower = el->pivots.begin();
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

void Dimension::write_pivtos(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, binned_container& subset) {
   for (const auto& el : subset) {
      auto it_lower = el->pivots.begin();
      for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
         if (!search_iterators(it_range, range, it_lower, el->pivots)) break;
         while (it_lower != el->pivots.end() && (*it_range).pivot >= (*it_lower)) {
            writer.StartArray();
            writer.Uint((*it_lower).front());
            writer.Uint((*it_lower++).back());
            writer.EndArray();
         }
      }
   }   
}
