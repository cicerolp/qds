#include "stdafx.h"
#include "NDS.h"
#include "Spatial.h"

Spatial::Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
   : Dimension(tuple), _container(spatial_t(0, 0, 0)) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {

   uint32_t pivots_count = 0;

   _container.set_range(range);
   pivots_count += _container.expand(data, response, _offset);

   std::sort(response.begin(), response.end());

   return pivots_count;
}

bool Spatial::query(const Query& query, range_container& range, response_container& response, binned_container& subset, const Dimension* target) const {

   if (query.eval_tile(_key)) {
      _container.query_tile(query, subset, 0);
   } else if (query.eval_region(_key)) {
      _container.query_region(query, subset, 0);
   } else {
      return false;
   }

   if (target != nullptr) {
      //restrict(range, response, subset, CopyValueFromRange);

      //std::cout << serialize<spatial_t, std::unordered_map>(response, subset, CopyValueFromSubset) << std::endl;

   } else if (query.type() == Query::TILE) {
      target = this;
      //restrict(range, response, subset, CopyValueFromSubset);

      std::cout << serialize(query, response, subset) << std::endl;

   } else if (query.type() == Query::GROUP) {
      target = this;
      //restrict(range, response, subset, DefaultCopy);

   } else {
      restrict(range, response, subset, DefaultCopy);
   }

   return true;
}

std::string Spatial::serialize(const Query& query, range_container& range, binned_container& subset) const {
   // sort range only when necessary
   std::sort(range.begin(), range.end());

   if (range.size() == 0) return std::string();

   pivot_iterator it_lower, it_upper;

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   // start json
   writer.StartArray();

   CopyOption option = CopyValueFromSubset;

   switch (query.type()) {
      case Query::TILE: {
         std::unordered_map<uint64_t, uint32_t> map;

         for (const auto& el : subset) {
            it_lower = el->pivots.begin();
            for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
               if (!search_iterators(it_range, range, it_lower, it_upper, el->pivots)) break;

               if (option == CopyValueFromRange) {
                  while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                     map[(*it_range).value] += (*it_upper).size();
                     ++it_upper;
                  }
               } else if (option == CopyValueFromSubset) {
                  while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                     map[el->value] += (*it_upper).size();
                     ++it_upper;
                  }
               }

               it_lower = it_upper;
            }
         }

         // serialization
         for (const auto& pair : map) {
            writer.StartArray();
            (*(spatial_t*)&pair.first).serialize(writer);
            writer.Uint(pair.second);
            writer.EndArray();
         }
      }
         break;

      case Query::REGION: {
         uint32_t count = 0;

         for (const auto& el : subset) {
            it_lower = el->pivots.begin();
            for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
               if (!search_iterators(it_range, range, it_lower, it_upper, el->pivots)) break;

               while (it_upper != el->pivots.end() && (*it_range).pivot >= (*it_upper)) {
                  count += (*it_upper).size();
                  ++it_upper;
               }

               it_lower = it_upper;
            }
         }

         // serialization
         writer.StartArray();
         writer.Uint(count);
         writer.EndArray();
      }
         break;
      default: break;
   }

   // end json
   writer.EndArray();
   return buffer.GetString();
}
