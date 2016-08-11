#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
public:
	Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
	~Spatial() = default;

	uint32_t build(const building_container& range, building_container& response, Data& data) override;
   bool query(const Query& query, range_container& range, range_container& response, binned_container& subset, binned_container& subset_exp, CopyOption& option) const override;

private:
   SpatialElement _container;
};
