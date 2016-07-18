#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"

class SpatialElement {
public:
   SpatialElement(const spatial_t& tile);
   ~SpatialElement() = default;

   uint32_t build(const Pivot& pivot, Data& data);
   void query(const Query& query, std::vector<const SpatialElement*>& subset) const;

   inline const spatial_t& tile() const {
      return _tile;
   }
   inline const std::vector<Pivot>& pivots() const {
      return _pivots;
   }

private:
   uint32_t expand(Data& data);

   void aggregate_tile(const Query& query, std::vector<const SpatialElement*>& subset) const;

   static const uint32_t max_levels{ 25 };

   spatial_t _tile;
   std::vector<Pivot> _pivots;
   std::array<std::unique_ptr<SpatialElement>, 4> _container;   
};
