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

   //TODO
   //current = cube->root();
   current.emplace_back(0, data.size());

   // BUG fix root pivot
   _root = BinnedPivot(Pivot(0, data.size()), 0);

   // categorical
   for (const auto& tuple : schema.categorical) {

      std::cout << "\tBuilding Categorical Dimension: " + std::get<0>(tuple) + " ... ";

      _categorical.emplace_back(std::make_unique<Categorical>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)));

      uint32_t curr_count = _categorical.back()->build(current, expand, data);
      pivots_count += curr_count;

      current.swap(expand);
      expand.clear();

      std::cout << "OK. \n\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   // temporal
   for (const auto& tuple : schema.temporal) {

      std::cout << "\tBuilding Temporal Dimension: " + std::get<0>(tuple) + " ... ";

      _temporal.emplace_back(std::make_unique<Temporal>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)));

      uint32_t curr_count = _temporal.back()->build(current, expand, data);
      pivots_count += curr_count;

      current.swap(expand);
      expand.clear();

      std::cout << "OK. \n\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   // spatial
   // BUG fix multiple spatial dimensions
   for (const auto& tuple : schema.spatial) {

      std::cout << "\tBuilding Spatial Dimension: " + std::get<0>(tuple) + " ... ";

      _spatial = std::make_unique<Spatial>(std::get<0>(tuple), schema.leaf, std::get<1>(tuple));

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

std::string NDS::query(const Query& query) {

   response_container range, response;
   range.emplace_back(_root);

   for (const auto& d : _categorical) {
      if (d->query(query, range, response)) {
         swap_and_sort(range, response);
      }
   }

   for (const auto& d : _temporal) {
      if (d->query(query, range, response)) {
         swap_and_sort(range, response);
      }
   }

   if (_spatial->query(query, range, response)) {
      swap_and_sort(range, response);
   }

   rapidjson::StringBuffer buffer;
   rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

   int count = 0;

   writer.StartArray();
   for (const auto& ptr : range) {
      writer.String(static_cast<std::string>(ptr.pivot).c_str());

      count += ptr.pivot.size();
   }

   writer.Uint(count);
   writer.EndArray();

   return buffer.GetString();
}
