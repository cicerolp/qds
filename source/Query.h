#pragma once

#include <ostream>
#include "string_util.h"
#include "types.h"

class Query {
 public:
  using const_t = std::pair<std::string, std::string>;
  using group_t = bool;

  Query(const std::string &url);

  inline const const_t* get_const(const std::string &dim) const {
    auto pair = _constraints.find(dim);

    if (pair == _constraints.end()) {
      return nullptr;
    } else {
      return &(pair->second);
    }
  };

  inline group_t get_group(const std::string &dim) const {
    return _group_by == dim;
  };

  inline bool has_group() const {
    return !_group_by.empty();
  }

  const std::string& get_instance() const {
    return _dataset;
  }

  friend std::ostream &operator<<(std::ostream &os, const Query &query);

 private:
  void parse(const std::string &url);

  std::string _url;

  std::string _dataset;

  // [dimesion name] -> constraints
  std::unordered_map<std::string, const_t> _constraints;

  // [dimesion name] -> group_by
  std::string _group_by;

  // [dimesion name] -> aggr
  std::pair<std::string, std::string> _aggr;
};

