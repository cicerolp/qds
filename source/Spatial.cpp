#include "stdafx.h"
#include "Spatial.h"

Spatial::Spatial(const std::string& key, const uint32_t bin, const uint8_t offset) 
   : Dimension(key, bin, offset) { }

uint32_t Spatial::build(const building_container& range, building_container& response, Data& data) {
   throw std::logic_error("The method or operation is not implemented.");
}
