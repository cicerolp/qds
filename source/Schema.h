#pragma once

#include "stdafx.h"

struct Schema {
   Schema(const std::string filename) {
      boost::property_tree::ptree pt;

      try {
         boost::property_tree::read_xml(filename, pt);

         name = pt.get<std::string>("config.name");
         bytes = pt.get<uint8_t>("config.bytes");
         file = std::string(std::getenv("NDS_DATA")) + "\\" + pt.get<std::string>("config.file");

         for (auto& v : pt.get_child("config.schema")) {
            uint32_t index = v.second.get<uint32_t>("index");
            uint32_t bin = v.second.get<uint32_t>("bin");
            uint32_t offset = v.second.get<uint32_t>("offset");

            if (v.first == "spatial") {
               spatial.emplace_back(std::make_tuple(index, bin, offset));
            } else if (v.first == "categorical") {
               categorical.emplace_back(std::make_tuple(index, bin, offset));
            } else if (v.first == "temporal") {
               temporal.emplace_back(std::make_tuple(index, bin, offset));
            }
         }
      } catch (...) {
         std::cerr << "error: invalid schema file [" + filename + "]" << std::endl;
         std::abort();
      }
   }

   std::string name, file;
   uint8_t bytes, leaf;

   // index, bin, offset
   std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> spatial;
   // index, bin, offset
   std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> categorical;
   // index, bin, offset
   std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> temporal;
};
