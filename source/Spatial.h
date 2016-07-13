#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
public:
	Spatial(const std::string& key, const uint32_t bin, const uint8_t offset);
	~Spatial() = default;

	virtual uint32_t build(const building_container& range, building_container& response, Data& data) override;

private:
   SpatialElement _container;
};
