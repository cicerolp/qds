#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
public:
	Spatial(const std::string& key, const uint32_t bin, const uint8_t offset);
	~Spatial() = default;

	uint32_t build(const building_container& range, building_container& response, Data& data) override;
   bool query(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const override;

private:
   SpatialElement _container;
};
