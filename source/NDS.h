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

   std::string query(const Query& query);

private:   
   std::vector<std::unique_ptr<Categorical>> _categorical;
   std::vector<std::unique_ptr<Temporal>> _temporal;
   std::unique_ptr<Spatial> _spatial;

   BinnedPivot _root;
};