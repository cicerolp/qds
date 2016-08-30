#include "stdafx.h"
#include "NDS.h"

#include "Spatial.h"
#include "Temporal.h"
#include "Categorical.h"

NDS::NDS(const Schema& schema) {

   std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
   start = std::chrono::high_resolution_clock::now();

   uint32_t pivots_count = 0;

   _data_ptr = std::make_unique<Data>(schema.file);

   std::cout << "Buildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << _data_ptr->size() << std::endl;
   std::cout << std::endl;

   _root = pivot_ctn(1);
   _root[0] = Pivot(0, _data_ptr->size());

   link_ctn links, share;
   links.emplace_back(&_root);

   build_ctn current, response;
   current.emplace_back(0, _data_ptr->size());

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

      uint32_t curr_count = _dimension.back().second->build(current, response, links, share, *this);
      pivots_count += curr_count;

      swap_and_clear<link_ctn>(links, share);
      swap_and_clear<build_ctn>(current, response);

      std::cout << "\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
   }
   
   std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

   end = std::chrono::high_resolution_clock::now();
   long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

   std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;

   // release data
   _data_ptr = nullptr;
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
   
   std::string buffer = Dimension::serialize(query, subsets, BinnedPivot(_root[0]));

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
