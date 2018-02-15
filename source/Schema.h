#pragma once

#include "stdafx.h"
#include "Dimension.h"

struct Schema {
  Schema(const std::string filename) {
    boost::property_tree::ptree pt;

    const std::string ENV_DIR = std::string(std::getenv("NDS_DATA"));

    try {
      boost::property_tree::read_xml(filename, pt);

      name = pt.get<std::string>("config.name");
      //bytes = pt.get<uint8_t>("config.bytes");

      auto pt_files = pt.get_child("config.files", boost::property_tree::ptree());

      if (!pt_files.empty()) {
        // multiple files
        for (auto &file : pt_files) {
          files.emplace_back(ENV_DIR + "/" + file.second.data());
        }
      } else {
        //single file
        files.emplace_back(ENV_DIR + "/" + pt.get<std::string>("config.file", ""));
      }

      for (auto &v : pt.get_child("config.schema")) {
        std::string index = v.second.get<std::string>("index");
        uint32_t bin = v.second.get<uint32_t>("bin");
        uint32_t offset = v.second.get<uint32_t>("offset");

        if (v.first == "spatial") {
          dimension.emplace_back(DimensionSchema::Spatial, index, bin, offset);
        } else if (v.first == "categorical") {
          dimension.emplace_back(DimensionSchema::Categorical, index, bin, offset);
        } else if (v.first == "temporal") {
          dimension.emplace_back(DimensionSchema::Temporal, index, bin, offset);
        } else if (v.first == "payload") {
          payload.emplace_back(DimensionSchema::Payload, index, bin, offset);
        }
      }
    } catch (...) {
      std::cerr << "error: invalid schema file [" + filename + "]" << std::endl;
      std::abort();
    }
  }

  std::string name;
  std::vector<std::string> files;

  // payload [index, bin, offset]
  std::vector<DimensionSchema> payload;

  // dimension_e, index, bin, offset
  std::vector<DimensionSchema> dimension;
};
