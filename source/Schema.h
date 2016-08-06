#pragma once

#include "stdafx.h"
#include "Dimension.h"

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
               dimension.emplace_back(std::make_tuple(Dimension::Spatial, index, bin, offset));
            } else if (v.first == "categorical") {
               dimension.emplace_back(std::make_tuple(Dimension::Categorical, index, bin, offset));
            } else if (v.first == "temporal") {
               dimension.emplace_back(std::make_tuple(Dimension::Temporal, index, bin, offset));
            }
         }
      } catch (...) {
         std::cerr << "error: invalid schema file [" + filename + "]" << std::endl;
         std::abort();
      }
   }

   uint8_t bytes;
   std::string name, file;

   // dimension_e, index, bin, offset
   std::vector<std::tuple<Dimension::Type, uint32_t, uint32_t, uint32_t>> dimension;
};
