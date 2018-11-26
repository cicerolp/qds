#pragma once
#include "types.h"
#include "Query.h"

class GeoCtnIntf {
 public:
  GeoCtnIntf() = default;

  virtual ~GeoCtnIntf() = default;

  // build container
  virtual void create() = 0;

  // update container
  virtual void insert(const std::string &filename) = 0;

  virtual void query(const Query &query) = 0;

  inline virtual std::string name() const;
};

std::string GeoCtnIntf::name() const {
  static auto name_str = "Unknown";
  return name_str;
}
