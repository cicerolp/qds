#pragma once
#include "types.h"
#include "Query.h"

class GeoCtnIntf {
 public:
  GeoCtnIntf() = default;

  virtual ~GeoCtnIntf() = default;

  // snap
  virtual void create_snap() = 0;
  virtual void insert_snap(const std::string &filename) = 0;

  // on-time
  virtual void create_on_time() = 0;
  virtual void insert_on_time(const std::string &filename) = 0;

  // small-twitter
  virtual void create_small_twitter() = 0;
  virtual void insert_small_twitter(const std::string &filename) = 0;

  // gaussian
  virtual void create_gaussian() = 0;
  virtual void insert_gaussian(const std::string &filename) = 0;

  virtual void query(const Query &query) = 0;

  inline virtual std::string name() const;
};

std::string GeoCtnIntf::name() const {
  static auto name_str = "Unknown";
  return name_str;
}
