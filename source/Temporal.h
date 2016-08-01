#pragma once

#include "types.h"
#include "Dimension.h"

class Temporal : public Dimension {
   struct TemporalElement {
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

      temporal_t date;
      pivot_container container;
   };
public:
	Temporal(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
   ~Temporal() = default;

   uint32_t build(const building_container& range, building_container& response, Data& data) override;
   bool query(const Query& query, range_container& range, response_container& response, bool& pass_over_target) const override;

   inline interval_t get_interval() const {
      return interval_t(_container.front().date, _container.back().date);
   }

private:
   stde::dynarray<TemporalElement> _container;
};