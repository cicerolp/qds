#pragma once

#include "Data.h"
#include "Pivot.h"

class Dimension {
public:
   Dimension(const std::string& key, const uint32_t bin, const uint8_t offset)
      : _key(key), _bin(bin), _offset(offset){ }
	virtual ~Dimension() = default;

   virtual uint32_t build(const building_container& range, building_container& response, Data& data) = 0;

protected:
   const std::string _key;
   const uint32_t _bin;
   const uint8_t _offset;
};