#pragma once

#include "types.h"
#include "Dimension.h"

class Temporal : public Dimension {
   struct TemporalElement {
      bool operator!=(const TemporalElement& rhs) const {
         return el.value != rhs.el.value;
      }
      bool operator==(const TemporalElement& rhs) const {
         return el.value == rhs.el.value;
      }
      bool operator<(const temporal_t& rhs) const {
         return (temporal_t)(el.value) < rhs;
      }
      bool operator<(const TemporalElement& rhs) const {
         return el.value < rhs.el.value;
      }

      binned_t el;
   };
public:
	Temporal(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
   ~Temporal() = default;

   uint32_t build(const building_container& range, building_container& response, NDS& nds) override;

   bool query(const Query& query, subset_container& subsets) const override;

   inline interval_t get_interval() const {
      return interval_t(_container.front().el.value, _container.back().el.value);
   }

   
private:
   stde::dynarray<TemporalElement> _container;
};