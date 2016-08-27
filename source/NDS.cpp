#include "stdafx.h"
#include "NDS.h"

#include "Spatial.h"
#include "Temporal.h"
#include "Categorical.h"

NDS::NDS(const Schema& schema) {

   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
   start = std::chrono::high_resolution_clock::now();

   uint32_t pivots_count = 0;

   data_ptr = std::make_unique<Data>(schema.file);

   pivots.emplace_back(pivot_container(1));
   pivots[0][0] = Pivot(0, data_ptr->size());

   std::cout << "\nBuildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << data_ptr->size() << std::endl;

   std::cout << std::endl;

   building_container current, response;
   current.emplace_back(0, data_ptr->size());

   for (const auto& tuple : schema.dimension) {

      auto opt = std::make_tuple(std::get<1>(tuple), std::get<2>(tuple), std::get<3>(tuple));

      switch (std::get<0>(tuple)) {
         case Dimension::Spatial:
            std::cout << "\tBuilding Spatial Dimension: " << std::get<1>(tuple) << std::endl;
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Spatial>(opt)));
            break;
         case Dimension::Temporal:
            std::cout << "\tBuilding Temporal Dimension: " << std::get<1>(tuple) << std::endl;
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Temporal>(opt)));
            break;
         case Dimension::Categorical:
            std::cout << "\tBuilding Categorical Dimension: " << std::get<1>(tuple) << std::endl;
            _dimension.emplace_back(std::make_pair(std::get<0>(tuple), std::make_unique<Categorical>(opt)));
            break;
         default:
            std::cerr << "error: invalid NDS" << std::endl;
            std::abort();
            break;
      }

      uint32_t curr_count = _dimension.back().second->build(current, response, *this);
      pivots_count += curr_count;

      current.swap(response);
      response.clear();

      std::cout << "\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }

   std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

   end = std::chrono::high_resolution_clock::now();
   long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

   std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;

   // release data
   data_ptr = nullptr;
}

std::string NDS::query(const Query& query, std::ofstream* telemetry) {

   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
   
   subset_container subsets;   

   start = std::chrono::high_resolution_clock::now();

   for (auto& pair : _dimension) {
      if (!pair.second->query(query, subsets)) {
         subsets.clear();
         break;
      }
   }
   
   std::string buffer = Dimension::serialize(query, subsets, BinnedPivot(pivots[0].front()));

   end = std::chrono::high_resolution_clock::now();

   if (telemetry != nullptr) {
      auto clock = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      (*telemetry) << clock << "," << query << std::endl;
   }

   return buffer;
}

interval_t NDS::get_interval() const {
   interval_t interval;
   for (auto& pair : _dimension) {
      if (pair.first == Dimension::Temporal) {
         interval = ((Temporal*)pair.second.get())->get_interval();
         break;
      }
   }
   return interval;
}
