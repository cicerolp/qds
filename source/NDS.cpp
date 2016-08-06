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

   for (const auto& tuple : schema.dimension) {

      auto opt = std::make_tuple(std::get<1>(tuple), std::get<2>(tuple), std::get<3>(tuple));

      switch (std::get<0>(tuple)) {
         case Dimension::Spatial:
            std::cout << "\tBuilding Spatial Dimension: " << std::get<1>(tuple) << " ... ";
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Spatial>(opt)));
            break;
         case Dimension::Temporal:
            std::cout << "\tBuilding Temporal Dimension: " << std::get<1>(tuple) << " ... ";
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Temporal>(opt)));
            break;
         case Dimension::Categorical:
            std::cout << "\tBuilding Categorical Dimension: " << std::get<1>(tuple) << " ... ";
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Categorical>(opt)));
            break;
         default:
            std::cerr << "error: invalid NDS" << std::endl;
            std::abort();
            break;
      }

      uint32_t curr_count = _dimension.back().second->build(current, expand, data);
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

   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

   Dimension::CopyOption option = Dimension::DefaultCopy;

   std::string buffer;
   binned_container subset;
   range_container range, response;

   response.emplace_back(_root);

   start = std::chrono::high_resolution_clock::now();

   for (auto& pair : _dimension) {
      pair.second->query(query, range, response, subset, option);
   }

   buffer = Dimension::serialize(query, range, response, subset, option);

   end = std::chrono::high_resolution_clock::now();

   if (telemetry != nullptr) {
      auto clock = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      (*telemetry) << clock << "," << query << std::endl;
   }

   return buffer;
}
