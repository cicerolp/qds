#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"
#include "Dimension.h"


class NDS {
public:
	NDS(const Schema& schema);
	~NDS() = default;

   std::string query(const Query& query, std::ofstream* telemetry);

   interval_t get_interval() const;

   inline uint32_t size() const {
      return pivots[0].front().back();
   }

   inline void share(binned_t &binned, const building_container& container) {
      pivots.emplace_back(container.size());
      std::memcpy(&pivots.back()[0], &container[0], container.size() * sizeof(Pivot));
      binned.pivots = &pivots.back();
   }

   inline Data* data() const {
      return data_ptr.get();
   }

private:
   std::unique_ptr<Data> data_ptr;
   std::vector<pivot_container> pivots;
   std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>> _dimension;
};