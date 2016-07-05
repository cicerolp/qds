#include "stdafx.h"
#include "NDS.h"

NDS::NDS(const Schema& schema) {
   
   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
   start = std::chrono::high_resolution_clock::now();

   uint32_t pivots_count = 0;

	Data data(schema.file);

   std::cout << "\nBuildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << data.size()  << std::endl;

   std::cout << std::endl;

   building_container current, expand;

   //TODO
   //current = cube->root();
   current.emplace_back(0, data.size());

   // categorical
   for (const auto& tuple : schema.categorical) {

      std::cout << "\tBuilding Categorical Dimension: " + std::get<0>(tuple) << std::endl;

	   _categorical.emplace(std::get<0>(tuple), std::make_unique<CategoricalDimension>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple)));

      uint32_t curr_count = _categorical[std::get<0>(tuple)]->build(current, expand, data);

      current.swap(expand);
      expand.clear();

      pivots_count += curr_count;
   }

   std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

   end = std::chrono::high_resolution_clock::now();
   long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

   std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;
}

std::string NDS::query(const Query& query) {
   return " ";
}
