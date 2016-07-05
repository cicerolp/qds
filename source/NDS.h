#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"

#include "CategoricalDimension.h"

class NDS {
public:
	NDS(const Schema& schema);
	~NDS() = default;

   std::string query(const Query& query);

private:
   std::map<std::string, std::unique_ptr<CategoricalDimension>> _categorical;
};