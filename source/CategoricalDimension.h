#pragma once

#include "Data.h"
#include "Pivot.h"

class CategoricalDimension {
public:
	CategoricalDimension(const std::string& key, const uint8_t bin, const uint8_t offset);
	virtual ~CategoricalDimension();
	
	uint32_t build(const building_container& range, building_container& response, Data& data);

private:
	const std::string _key;
	const uint8_t _bin{ 0 }, _offset{ 0 };

	std::unordered_map<std::string, std::vector<Pivot>> _container;
};