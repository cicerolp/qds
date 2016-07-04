#include "stdafx.h"
#include "NDS.h"

NDS::NDS(const Schema& schema) {
   
   _data = std::make_unique<Data>(schema.file);

   std::cout << "\nBuildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << data->size() << std::endl;

   building_container current, expand;
   current = cube->root();

   start = std::chrono::high_resolution_clock::now();

   // spatial
   pivots_count += cube->setSpatial(schema.spatial, schema.leaf)->hash(current, response, *data.get());
   current.swap(response);
   response.clear();
   std::cout << "\t\tNumber of Pivots: " << current.size() << std::endl;

   // categorical
   for (auto& pair : schema.categorical) {
      pivots_count += cube->addCategorical(pair.first, pair.second)->hash(current, response, *data.get());
      current.swap(response);
      response.clear();
      std::cout << "\t\tNumber of Pivots: " << current.size() << std::endl;
   }

   // temporal
   for (auto& pair : schema.temporal) {
      pivots_count += cube->addTemporal(pair.first, pair.second)->hash(current, response, *data.get());
      current.swap(response);
      response.clear();
      std::cout << "\t\tNumber of Pivots: " << current.size() << std::endl;
   }

   end = std::chrono::high_resolution_clock::now();
   long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

   std::cout << "\n\tDuration: " + std::to_string(duration) + "s\n" << std::endl;

   data->dispose();

   if (schema.loader == "sql") {
      cube->setSQLData(data);
   }
   return std::make_tuple(cube, duration, pivots_count);
}

NDS::~NDS() {

}

std::string NDS::query(const Query& query) {
   return " ";
}
