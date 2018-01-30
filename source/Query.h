#pragma once

#include <ostream>
#include "string_util.h"
#include "types.h"

class Query {
 public:
  using clausule = std::pair<std::string, std::string>;

  Query(const std::string &url);

  inline const std::string &get_dataset() const {
    return _dataset;
  }

  inline const std::vector<clausule> &get_aggr() const {
    return _aggr;
  }

  inline const clausule *get_const(const std::string &dim) const {
    auto pair = _constraints.find(dim);

    if (pair == _constraints.end()) {
      return nullptr;
    } else {
      return &(pair->second);
    }
  };

  inline bool group_by() const {
    return !_group_by.empty();
  };

  inline bool group_by(const std::string &dim) const {
    return _group_by == dim;
  };

  friend std::ostream &operator<<(std::ostream &os, const Query &query);

 private:
  void parse(const std::string &url);

  std::string _url;

  std::string _dataset;

  // [dimesion name] -> constraints
  std::unordered_map<std::string, clausule> _constraints;

  // [dimesion name] -> group_by
  std::string _group_by;

  // [aggr]
  std::vector<clausule> _aggr;
};

