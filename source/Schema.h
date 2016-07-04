#pragma once

#include "stdafx.h"
#include "types.h"

struct Schema {
   Schema(const std::string file) {
      boost::property_tree::ptree pt;
      boost::property_tree::read_xml(file, pt);

      name = pt.get<std::string>("config.name");
      bytes = pt.get<uint8_t>("config.bytes");

      for (auto &v : pt.get_child("config.schema")) {
         if (v.first == "spatial") {
            std::string key = v.second.get<std::string>("key");
			uint8_t offset = v.second.get<uint8_t>("offset");
            spatial.emplace_back(key, offset);

         } else {
            std::string key = v.second.get<std::string>("key");
			uint8_t bin = v.second.get<uint8_t>("bin");
			uint8_t offset = v.second.get<uint8_t>("offset");

            if (v.first == "categorical") {
               categorical.emplace_back(key, bin, offset);
            } else if (v.first == "temporal") {
               temporal.emplace_back(key, bin, offset);
            }
         }
      }
   }

   std::string name;
   uint32_t bytes;

   std::vector<std::tuple<std::string, uint8_t>> spatial;
   std::vector<std::tuple<std::string, uint8_t, uint8_t>> categorical;
   std::vector<std::tuple<std::string, uint8_t, uint8_t>> temporal;
};
