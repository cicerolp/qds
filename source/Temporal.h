#pragma once

#include "types.h"
#include "Dimension.h"

class Temporal : public Dimension {
   struct TemporalElement {
      TemporalElement(const std::pair<temporal_t, std::vector<Pivot>>& entry) :
         date(entry.first), container(entry.second) { }

      bool operator!=(const TemporalElement& rhs) const {
         return date != rhs.date;
      }
      bool operator==(const TemporalElement& rhs) const {
         return date == rhs.date;
      }
      bool operator<(const temporal_t& rhs) const {
         return date < rhs;
      }
      bool operator<(const TemporalElement& rhs) const {
         return date < rhs.date;
      }

      const temporal_t date;
      std::vector<Pivot> container;
   };
public:
	Temporal(const std::string& key, const uint32_t bin, const uint8_t offset);
   ~Temporal() = default;

   uint32_t build(const building_container& range, building_container& response, Data& data) override;
   bool query(const Query& query, const response_container& range, response_container& response, bool& pass_over_target) const override;

   inline interval_t get_interval() const {
      return interval_t(_container.front().date, _container.back().date);
   }

private:
   std::vector<TemporalElement> _container;
};