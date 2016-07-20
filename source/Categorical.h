#pragma once

#include "Dimension.h"

class Categorical : public Dimension {
   struct CategoricalElement {
      CategoricalElement(const categorical_t& value) :
         value(value) { }

      const categorical_t value;
      std::vector<Pivot> container;
   };

public:
   Categorical(const std::string& key, const uint32_t bin, const uint8_t offset);
   ~Categorical() = default;

   uint32_t build(const building_container& range, building_container& response, Data& data) override;
   bool query(const Query& query, response_container& range, response_container& response, bool& pass_over_target) const override;

private:
   bool query_field(const Query& query, response_container& range, response_container& response, bool& pass_over_target) const;
   bool query_where(const Query& query, response_container& range, response_container& response, bool& pass_over_target) const;

   std::vector<CategoricalElement> _container;
};
