#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
public:
   Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
   ~Categorical() = default;

   uint32_t build(const building_container& range, building_container& response, Data& data) override;
   void query(const Query& query, range_container& range, range_container& response, binned_container& subset, CopyOption& option) const override;

private:
   stde::dynarray<binned_t> _container;
};
