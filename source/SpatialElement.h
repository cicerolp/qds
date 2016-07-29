#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"

class SpatialElement {
   static const uint32_t max_levels{ 25 };

public:
   SpatialElement(const spatial_t& tile);
   ~SpatialElement() = default;

   template<class... Args>
   inline void add_element(Args&&... args);
   inline void add_range(const building_container& range);

   uint32_t expand(Data& data, const uint8_t offset);

   void query_tile(const Query& query, std::vector<const SpatialElement*>& subset) const;
   void query_region(const Query& query, std::vector<const SpatialElement*>& subset, uint8_t z) const;

   spatial_t value;
   std::vector<Pivot> pivots;

private:
   void aggregate_tile(const Query& query, std::vector<const SpatialElement*>& subset) const;

   std::array<std::unique_ptr<SpatialElement>, 4> _container;
};

template<class ... Args>
void SpatialElement::add_element(Args&&... args) {
   pivots.emplace_back(args...);
}

void SpatialElement::add_range(const building_container& range) {
   pivots.insert(pivots.end(), range.begin(), range.end());
}
