#pragma once

#include "stdafx.h"
#include "types.h"

struct Schema {
   Schema(const std::string filename) {
      boost::property_tree::ptree pt;

	  try {
		  boost::property_tree::read_xml(filename, pt);

		  name = pt.get<std::string>("config.name");
		  bytes = pt.get<uint8_t>("config.bytes");
		  file = pt.get<std::string>("config.file");

		  for (auto &v : pt.get_child("config.schema")) {
			  if (v.first == "spatial") {
				  std::string key = v.second.get<std::string>("key");
				  uint8_t offset = v.second.get<uint8_t>("offset");
				  spatial.emplace_back(key, offset);

			  }
			  else {
				  std::string key = v.second.get<std::string>("key");
				  uint8_t bin = v.second.get<uint8_t>("bin");
				  uint8_t offset = v.second.get<uint8_t>("offset");

				  if (v.first == "categorical") {
					  categorical.emplace_back(key, bin, offset);
				  }
				  else if (v.first == "temporal") {
					  temporal.emplace_back(key, bin, offset);
				  }
			  }
		  }
	  } catch (...) {
		  std::cerr << "error: invalid schema file <" + filename + ">" << std::endl;
	  }      
   }

   std::string name, file;
   uint32_t bytes;

   // key, offset
   std::vector<std::tuple<std::string, uint8_t>> spatial;
   // key, bin, offset
   std::vector<std::tuple<std::string, uint8_t, uint8_t>> categorical;
   // key, bin, offset
   std::vector<std::tuple<std::string, uint8_t, uint8_t>> temporal;
};
