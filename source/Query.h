#pragma once

#include "string_util.h"
#include "types.h"

class Query {
 public:
  enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION, QUANTILE };

  struct query_t {
    enum type { spatial, categorical, temporal };
    query_t(type _id) : id(_id) {}
    type id;
  };
  struct spatial_query_t : public query_t {
    spatial_query_t() : query_t(spatial) {}

    // TODO fix multiple spatial dimensions
    uint32_t resolution{0};
    std::vector<region_t> region;
    std::vector<spatial_t> tile;
  };
  struct categorical_query_t : public query_t {
    categorical_query_t() : query_t(categorical) {}

    bool field{false};
    std::vector<categorical_t> where;
  };
  struct temporal_query_t : public query_t {
    temporal_query_t() : query_t(temporal) {}

    interval_t interval;
  };

  using restriction_t = std::map<uint32_t, std::unique_ptr<query_t>>;

  Query(const std::string &url);
  Query(const std::vector<std::string> &tokens);

  inline const QueryType &type() const { return _type; }

  inline const std::string &instance() const { return _instance; }

  friend std::ostream &operator<<(std::ostream &os, const Query &query);

  template<typename T>
  inline T *eval(uint32_t key) const {
    auto pair = _restrictions.find(key);
    return pair == _restrictions.end() ? nullptr : (T *) pair->second.get();
  }

  const std::vector<float>& quantiles() const {
    return _quantiles;
  }

 private:
  Query(const std::string &instance, const std::string &type);

  template<typename T>
  inline T *get(uint32_t key) {
    auto pair = _restrictions.find(key);

    if (pair == _restrictions.end()) {
      _restrictions[key] = std::make_unique<T>();
      pair = _restrictions.find(key);
    }

    return (T *) pair->second.get();
  }

  QueryType _type;
  std::string _instance;

  std::vector<float> _quantiles;
  restriction_t _restrictions;
};