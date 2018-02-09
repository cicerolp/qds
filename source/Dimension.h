#pragma once

#include "RangePivot.h"
#include "Pivot.h"
#include "Query.h"
#include "Aggr.h"

class NDS;
class Data;

class Dimension {
 public:
  Dimension(const DimensionSchema &schema) : _schema(schema) {}
  virtual ~Dimension() = default;

  virtual bool query(const Query &query, subset_ctn &subsets) const = 0;

  virtual uint32_t build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) = 0;

 protected:
  const DimensionSchema _schema;
};
