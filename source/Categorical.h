#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
public:
   Categorical(const std::tuple<uint32_t, uint32_t, uint32_t>& tuple);
   ~Categorical() = default;

   uint32_t build(const building_container& range, building_container& response, NDS& nds) override;

   bool query(const Query& query, subset_container& subsets) const override;

private:
   stde::dynarray<binned_t> _container;
};
