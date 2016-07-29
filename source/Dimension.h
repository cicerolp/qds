#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   Dimension(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple)
      : _key(std::get<0>(tuple)), _bin(std::get<1>(tuple)), _offset(std::get<2>(tuple)){ }
	virtual ~Dimension() = default;

   virtual bool query(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

protected:
   const uint32_t _key;
   const uint32_t _bin;
   const uint32_t _offset;
};