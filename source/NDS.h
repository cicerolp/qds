#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"
#include "BinnedPivot.h"

#include "Spatial.h"
#include "Temporal.h"
#include "Categorical.h"

class NDS {
public:
	NDS(const Schema& schema);
	~NDS() = default;

   std::string query(const Query& query, std::ofstream* telemetry);

   inline interval_t get_interval() const {
      interval_t interval;
      for (auto& pair : _dimension) {
         if (pair.first == Dimension::Temporal) {
            interval = ((Temporal*)pair.second.get())->get_interval();
            break;
         }
      }
      return interval;
   }
   inline uint32_t size() const {
      return _root.pivot.back() - _root.pivot.front();
   }

private:

   BinnedPivot _root;
   std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>> _dimension;
};