#pragma once

#include "Dimension.h"

class Temporal : public Dimension {
   struct TemporalElement {
      TemporalElement(const std::pair<temporal_t, std::vector<Pivot>>& entry) :
         date(entry.first), container(entry.second) { }

      bool operator!= (const TemporalElement& rhs) const {
         return date != rhs.date;
      }
      bool operator== (const TemporalElement& rhs) const {
         return date == rhs.date;
      }

      temporal_t date;
      std::vector<Pivot> container;
   };
public:
	Temporal(const std::string& key, const uint32_t bin, const uint8_t offset);
	
   uint32_t build(const building_container& range, building_container& response, Data& data) override;

private:
   std::vector<TemporalElement> _container;
};