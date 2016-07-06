#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
public:
	Categorical(const std::string& key, const uint32_t bin, const uint8_t offset);
	
	uint32_t build(const building_container& range, building_container& response, Data& data) override;

private:
	std::unordered_map<std::string, std::vector<Pivot>> _container;
};