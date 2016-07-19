#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "BinnedPivot.h"

class Dimension {
public:
   Dimension(const std::string& key, const uint32_t bin, const uint8_t offset)
      : _key(key), _bin(bin), _offset(offset){ }
	virtual ~Dimension() = default;

   virtual bool query(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const = 0;
   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

protected:
   const std::string _key;
   const uint32_t _bin;
   const uint8_t _offset;
};