//
// Created by cicerolp on 11/17/17.
//

#pragma once

#include "Pivot.h"

using json = rapidjson::Writer<rapidjson::StringBuffer>;

class Aggr {
 protected:
  virtual inline void write_value(uint64_t value, json &writer) const {
    // TODO remove type hack
    if (value >= std::numeric_limits<uint32_t>::max()) {
      // spatial_t
      writer.Uint((*((spatial_t *) &value)).x);
      writer.Uint((*((spatial_t *) &value)).y);
      writer.Uint((*((spatial_t *) &value)).z);
    } else {
      // any other data type
      writer.Uint(value);
    }
  }
};

/////////////////////////////////////////////////////////////////////////////////////////
// Subset
/////////////////////////////////////////////////////////////////////////////////////////

class SubsetAggr : public Aggr {
 public:
  virtual void merge(size_t el, pivot_it &it_lower, pivot_it &it_upper) = 0;
  virtual void output(size_t el, uint64_t value, const Query &query, json &writer) = 0;
};

class CountSubsetAggr : public SubsetAggr {
 public:
  CountSubsetAggr(size_t size) : _map(size, 0) {}

  virtual void merge(size_t el, pivot_it &it_lower, pivot_it &it_upper) override {
    while (it_lower != it_upper) {
      _map[el] += (*it_lower++).size();
    }
  }

  virtual void output(size_t el, uint64_t value, const Query &query, json &writer) override {
    if (_map[el] == 0) return;

    writer.StartArray();
    write_value(value, writer);
    writer.Uint(_map[el]);
    writer.EndArray();
  }

 protected:
  std::vector<uint32_t> _map;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Range
/////////////////////////////////////////////////////////////////////////////////////////

class RangeAggr : public Aggr {
 public:
  virtual void merge(uint64_t value, pivot_it &it_lower, pivot_it &it_upper) = 0;
  virtual void output(const Query &query, json &writer) = 0;
};

class CountRangeAggr : public RangeAggr {
 public:
  virtual void merge(uint64_t value, pivot_it &it_lower, pivot_it &it_upper) override {
    while (it_lower != it_upper) {
      _map[value] += (*it_lower++).size();
    }
  }

  virtual void output(const Query &query, json &writer) override {
    for (const auto &pair : _map) {
      writer.StartArray();
      write_value(pair.first, writer);
      writer.Uint(pair.second);
      writer.EndArray();
    }
  }

 protected:
  std::map<uint64_t, uint32_t> _map;
};
