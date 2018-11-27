#pragma once

#include <ostream>
#include "source/util/string_util.h"
#include "types.h"

class Query {
 public:
  using clausule = std::pair<std::string, std::string>;
  using aggr_expr = std::pair<std::string, clausule>;

  Query() = default;
  Query(const std::string &url);

  void parse(const std::string &url);

  inline const std::string &get_dataset() const {
    return _dataset;
  }

  inline const std::vector<aggr_expr> &get_aggr() const {
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

  inline const std::string& get_group_by() const {
    return _group_by;
  };

  inline bool group_by() const {
    return !_group_by.empty();
  };

  inline bool group_by(const std::string &dim) const {
    return _group_by == dim;
  };

  std::string to_postgresql() const;

  std::string to_sqlite() const;

  std::string to_monetdb() const;

  friend std::ostream &operator<<(std::ostream &os, const Query &query);

 private:
  std::string _url;

  std::string _dataset;

  // [dimesion name] -> constraints
  std::unordered_map<std::string, clausule> _constraints;

  // [dimesion name] -> group_by
  std::string _group_by;

  // [aggr type, dimension, values]
  std::vector<aggr_expr> _aggr;
};

