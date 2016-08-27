#pragma once

#include "Dimension.h"
#include "SpatialElement.h"

class Spatial : public Dimension {
public:
	Spatial(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
	~Spatial() = default;

   uint32_t build(const building_container& range, building_container& response, NDS& nds) override;

   bool query(const Query& query, subset_container& subsets) const override;
   
private:
   SpatialElement _container;
};
