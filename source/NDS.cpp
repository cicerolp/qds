#include "stdafx.h"
#include "NDS.h"

NDS::NDS(const Schema& schema) {

   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
   start = std::chrono::high_resolution_clock::now();

   uint32_t pivots_count = 0;

   Data data(schema.file);

   std::cout << "\nBuildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << data.size() << std::endl;

   std::cout << std::endl;

   building_container current, expand;
   current.emplace_back(0, data.size());

   _root = BinnedPivot(Pivot(0, data.size()), 0);

   // categorical
   for (const auto& tuple : schema.categorical) {
      std::cout << "\tBuilding Categorical Dimension: " << std::get<0>(tuple) << " ... ";

      _categorical.emplace_back(std::make_unique<Categorical>(tuple));

      uint32_t curr_count = _categorical.back()->build(current, expand, data);
      pivots_count += curr_count;

      current.swap(expand);
      expand.clear();

      std::cout << "OK. \n\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   // temporal
   for (const auto& tuple : schema.temporal) {
      std::cout << "\tBuilding Temporal Dimension: " << std::get<0>(tuple) << " ... ";

      _temporal.emplace_back(std::make_unique<Temporal>(tuple));

      uint32_t curr_count = _temporal.back()->build(current, expand, data);
      pivots_count += curr_count;

      current.swap(expand);
      expand.clear();

      std::cout << "OK. \n\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   // spatial
   // TODO multiple spatial dimensions
   for (const auto& tuple : schema.spatial) {
      std::cout << "\tBuilding Spatial Dimension: " << std::get<0>(tuple) << " ... ";

      _spatial = std::make_unique<Spatial>(tuple);

      uint32_t curr_count = _spatial->build(current, expand, data);
      pivots_count += curr_count;

      current.swap(expand);
      expand.clear();

      std::cout << "OK. \n\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

   end = std::chrono::high_resolution_clock::now();
   long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

   std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;
}

std::string NDS::query(const Query& query, std::ofstream* telemetry) {

   std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> start(4), end(4);

   bool pass_over_target = false;

   binned_container subset;
   
   range_container range;
   response_container response;
   response.emplace_back(_root);

   // categorical
   start[0] = std::chrono::high_resolution_clock::now();
   for (const auto& d : _categorical) {
      d->query(query, range, response, subset, pass_over_target);
   }
   end[0] = std::chrono::high_resolution_clock::now();

   // temporal
   start[1] = std::chrono::high_resolution_clock::now();
   for (const auto& d : _temporal) {
      d->query(query, range, response, subset, pass_over_target);
   }
   end[1] = std::chrono::high_resolution_clock::now();

   // spatial
   start[2] = std::chrono::high_resolution_clock::now();
   _spatial->query(query, range, response, subset, pass_over_target);
   end[2] = std::chrono::high_resolution_clock::now();

   // serialization (json)
   start[3] = std::chrono::high_resolution_clock::now();
   std::string buffer(serialize(query, response));
   end[3] = std::chrono::high_resolution_clock::now();

   if (telemetry != nullptr) {
      auto clock_categorical = std::chrono::duration_cast<std::chrono::milliseconds>(end[0] - start[0]).count();
      auto clock_temporal = std::chrono::duration_cast<std::chrono::milliseconds>(end[1] - start[1]).count();
      auto clock_spatial = std::chrono::duration_cast<std::chrono::milliseconds>(end[2] - start[2]).count();
      auto clock_json = std::chrono::duration_cast<std::chrono::milliseconds>(end[3] - start[3]).count();

      auto clock = clock_categorical + clock_temporal + clock_spatial + clock_json;

      (*telemetry) << clock << "," << clock_spatial << "," << clock_categorical << "," << clock_temporal << "," << clock_json << "," << query << std::endl;
   }

   return buffer;
}

std::string NDS::serialize(const Query& query, const response_container& response) {

   // serialization
   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   writer.StartArray();

   switch (query.type()) {
      case Query::TILE: {
         std::unordered_map<uint64_t, uint32_t> map;
         for (const auto& ptr : response) map[ptr.value] += ptr.pivot.size();

         for (const auto& pair : map) {
            writer.StartArray();
            writer.Uint((*(spatial_t*)&pair.first).x);
            writer.Uint((*(spatial_t*)&pair.first).y);
            writer.Uint((*(spatial_t*)&pair.first).z);
            writer.Uint(pair.second);
            writer.EndArray();
         }
      }
         break;
      case Query::GROUP: {
         std::map<uint64_t, uint32_t> map;
         for (const auto& ptr : response) map[ptr.value] += ptr.pivot.size();

         for (const auto& pair : map) {
            writer.StartArray();
            writer.Uint((*(categorical_t*)&pair.first));
            writer.Uint(pair.second);
            writer.EndArray();
         }
      }
         break;
      case Query::TSERIES: {
         std::map<uint64_t, uint32_t> map;
         for (const auto& ptr : response) map[ptr.value] += ptr.pivot.size();

         for (const auto& pair : map) {
            writer.StartArray();
            writer.Uint((*(temporal_t*)&pair.first));
            writer.Uint(pair.second);
            writer.EndArray();
         }
      }
         break;
      case Query::SCATTER: {
         // TODO scatter serialization
      }
         break;
      case Query::MYSQL: {
         // TODO SQL serialization
      }
         break;
      case Query::REGION: {
         uint32_t count = 0;
         for (const auto& ptr : response) count += ptr.pivot.size();

         writer.StartArray();

         writer.Uint(count);
         writer.EndArray();
      }
         break;
      default: break;
   }

   writer.EndArray();

   return buffer.GetString();
}
