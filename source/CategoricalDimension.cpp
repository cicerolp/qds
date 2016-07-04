#include "stdafx.h"
#include "CategoricalDimension.h"

CategoricalDimension::CategoricalDimension(const std::string& key, const uint8_t bin, const uint8_t offset)
	: _key(key), _bin(bin), _offset(offset) { }

CategoricalDimension::~CategoricalDimension() { }

uint32_t CategoricalDimension::build(const response_container& range, response_container& response, Data& data) {

	uint32_t pivots_count = 0;

	for (const auto& ptr : range) {
		std::vector<categorical_t> used(_bin, 0);

		for (auto i = ptr.front(); i < ptr.back(); ++i) {
			categorical_t value = data.record<categorical_t>(i, _offset);

			data.setHash(i, value);
			used[value]++;
		}

		uint32_t accum = ptr.front();
		for (uint8_t i = 0; i < _bin; ++i) {

			if (used[i] == 0) continue;

			uint32_t first = accum;
			accum += used[i];
			uint32_t second = accum;

			_container[std::to_string(i)].emplace_back(first, second);

			response.emplace_back(first, second);
			pivots_count++;
		}		
	}

	data.sort(range.front().front(), range.back().back());
}
