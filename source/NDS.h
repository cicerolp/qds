#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"

class NDS {
public:
	NDS(const Schema& schema);
	~NDS();

   std::string query(const Query& query);

protected:
   std::unique_ptr<Data> _data;

private:
};