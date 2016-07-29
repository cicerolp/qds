#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"
#include "BinnedPivot.h"

#include "Spatial.h"
#include "Categorical.h"
#include "Temporal.h"

class NDS {
public:
	NDS(const Schema& schema);
	~NDS() = default;

   std::string query(const Query& query, std::ofstream* telemetry);

   inline interval_t get_interval() const {
      interval_t interval;
      if (_temporal.size() != 0) {
         interval = _temporal.back()->get_interval();
      }
      return interval;
   }
   inline uint32_t size() const {
      return _root.pivot.back() - _root.pivot.front();
   }

   static inline void swap_and_sort(range_container& range, response_container& response) {
      // vector, vector
      range.swap(response);
      std::sort(range.begin(), range.end());
      response.clear();

      // vector, deque
      /*range.assign(response.begin(), response.end());
      std::sort(range.begin(), range.end());
      response.clear();*/
   }

private:
   static std::string serialize(const Query& query, const response_container& response);

   std::vector<std::unique_ptr<Categorical>> _categorical;
   std::vector<std::unique_ptr<Temporal>> _temporal;
   std::unique_ptr<Spatial> _spatial;

   BinnedPivot _root;
};