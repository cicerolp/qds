#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"

#include "CategoricalDimension.h"

class NDS {
public:
	NDS(const Schema& schema);
	~NDS();

   std::string query(const Query& query);

private:
   std::unique_ptr<Data> _data;

   std::unordered_map<std::string, std::unique_ptr<CategoricalDimension>> _categorical;
};