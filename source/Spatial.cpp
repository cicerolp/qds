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

bool Spatial::query(const Query& query, range_container& range, response_container& response, CopyOption& option) const {

   if (!query.eval(_key)) return false;

   binned_container subset;
   auto restriction = query.get<Query::spatial_query_t>(_key);

   if (restriction->tile.size()) {
      _container.query_tile(restriction->tile, restriction->resolution, subset, 0);
   } else if (query.get<Query::spatial_query_t>(_key)->region.size()) {
      _container.query_region(restriction->region, subset, 0);
   } else {
      return false;
   }

   if (query.type() == Query::TILE) {
      restrict(range, response, subset, CopyValueFromSubset);
      option = CopyValueFromRange;
   } else {
      restrict(range, response, subset, option);
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

   if (query.type() == Query::TILE) {
      std::unordered_map<uint64_t, uint32_t> map;

      for (const auto& el : subset) {
         it_lower = el->pivots.begin();
         for (auto it_range = range.begin(); it_range != range.end(); ++it_range) {
            if (!search_iterators(it_range, range, it_lower, it_upper, el->pivots)) break;

            if (option == CopyValueFromRange) {
               while (it_lower != it_upper) {
                  map[(*it_range).value] += (*it_lower++).size();
               }
            } else if (option == CopyValueFromSubset) {
               while (it_lower != it_upper) {
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
            if (!search_iterators(it_range, range, it_lower, it_upper, el->pivots)) break;

            while (it_lower != it_upper) {
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
