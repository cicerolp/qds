#include "stdafx.h"
#include "Dimension.h"
#include "NDSInstances.h"

Dimension::Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
                    : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)) {
   std::cout << "\t\tKey: [" << _key  << "], Bin: [" << _bin << "], Offset: [" << _offset << "]" << std::endl;
}

void Dimension::restrict(range_container& range, range_container& response, const subset_t& subset, CopyOption& option) {

   if (option == DefaultCopy) option = subset.option;

   // sort range only when necessary
   swap_and_sort(range, response, option);

   for (const auto& el : subset.container) {
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

   if (option == CopyValueFromSubset) option = CopyValueFromRange;
}

std::string Dimension::serialize(const Query& query, subset_container& subsets, const BinnedPivot& root) {

   if (subsets.size() == 0) return std::string("[]");
   
   CopyOption option = DefaultCopy;
   range_container range, response;
   response.emplace_back(root);

   for (auto i = 0; i < subsets.size() - 1; ++i) {
      Dimension::restrict(range, response, subsets[i], option);
   }

   if (option == DefaultCopy) option = subsets.back().option;

   // sort range only when necessary
   swap_and_sort(range, response, option);

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   // start json
   writer.StartArray();

   switch (query.type()) {
      case Query::TILE: 
         if (option == CopyValueFromSubset) write_subset<spatial_t>(writer, range, subsets.back().container);
         else write_range<spatial_t, std::unordered_map>(writer, range, subsets.back().container);
         break;
      case Query::GROUP: 
         if (option == CopyValueFromSubset) {
            std::sort(subsets.back().container.begin(), subsets.back().container.end(), binned_t::Comp);
            write_subset<categorical_t>(writer, range, subsets.back().container);
         } else write_range<uint64_t, std::map>(writer, range, subsets.back().container);
         break;
      case Query::TSERIES: 
         if (option == CopyValueFromSubset) {
            std::sort(subsets.back().container.begin(), subsets.back().container.end(), binned_t::Comp);
            write_subset<temporal_t>(writer, range, subsets.back().container);
         } else write_range<uint64_t, std::map>(writer, range, subsets.back().container);
         break;
      case Query::SCATTER: break;
      case Query::MYSQL: break;
      case Query::REGION: 
         write_count(writer, range, subsets.back().container);
         break;
      default: break;
   }

   // end json
   writer.EndArray();
   return buffer.GetString();
}

void Dimension::write_count(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset) {
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

void Dimension::write_pivtos(rapidjson::Writer<rapidjson::StringBuffer>& writer, range_container& range, const binned_container& subset) {
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
