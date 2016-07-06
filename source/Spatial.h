#pragma once

#include "Dimension.h"

class Spatial : public Dimension {
   struct SpatialElement {
      uint64_t x : 29;
      uint64_t y : 29;
      uint64_t z : 5;
      uint64_t l : 1;
   };
public:
	Spatial(const std::string& key, const uint32_t bin, const uint8_t offset);
	~Spatial() = default;
	
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) override;

private:
   std::vector<std::vector<SpatialElement>> _container;
};