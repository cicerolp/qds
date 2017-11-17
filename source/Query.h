#pragma once

#include "string_util.h"
#include "types.h"

class Query {
 public:
  enum QueryOutput { COUNT, QUANTILE };
  enum QueryAggregation { NONE, TILE, GROUP, TSERIES, SCATTER };

  struct restriction_t {
    virtual ~restriction_t() = default;
    virtual inline void print(uint32_t id, std::ostream &os) const {
      // empty
    };
  };

  // TODO fix multiple spatial dimensions
  struct spatial_restriction_t : public restriction_t {
    std::unique_ptr<spatial_t> tile;
    std::unique_ptr<region_t> region;

    inline void print(uint32_t id, std::ostream &os) const override {
      if (tile != nullptr)
        os << "/tile/" << id << "/" << (*tile);
      if (region!= nullptr)
        os << "/region/" << id << "/" << (*region);
    };
  };

  struct categorical_restriction_t : public restriction_t {
    bool field{false};
    std::vector<categorical_t> where;

    inline void print(uint32_t id, std::ostream &os) const override {
      if (field) {
        os << "/field/" << id;
      }

      // /where/<category>=<[value]:[value]...:[value]>&<category>=<[value]:[value]...:[value]>
      if (where.size()) {
        os << "/where/" << id << "=";
        std::string where_stream;
        for (auto &value : where) {
          where_stream += std::to_string(value) + ":";
        }
        where_stream = where_stream.substr(0, where_stream.size() - 1);
        os << where_stream;
      }
    };
  };

  struct temporal_restriction_t : public restriction_t {
    interval_t interval;

    inline void print(uint32_t id, std::ostream &os) const override {
      os << "/tseries/" << id << "/" << interval;
    };
  };

  using restriction_map = std::map<uint32_t, std::unique_ptr<restriction_t>>;

  Query(const std::string &url);
  Query(const std::vector<std::string> &tokens);

  inline const QueryOutput &output() const { return _output; }
  inline const QueryAggregation &aggregation() const { return _aggregation; }

  inline const std::string &instance() const { return _instance; }

  template<typename T>
  inline T *eval(uint32_t key) const {
    auto pair = _restrictions.find(key);
    return pair == _restrictions.end() ? nullptr : (T *) pair->second.get();
  }

  const std::vector<float> &quantiles() const {
    return _quantiles;
  }

  void print(std::ostream &os) const;

 private:
  void initialize(const std::string &instance, const std::string &output, const std::string &aggregation);

  template<typename T>
  inline T *get(uint32_t key) {
    auto pair = _restrictions.find(key);

    if (pair == _restrictions.end()) {
      _restrictions[key] = std::make_unique<T>();
      pair = _restrictions.find(key);
    }

    return (T *) pair->second.get();
  }

  // dataset
  std::string _instance;

  std::string _aggregation_str;
  QueryAggregation _aggregation;

  // output type [count, quantile]
  std::string _output_str;
  QueryOutput _output;

  std::vector<float> _quantiles;

  // array of restrictions
  restriction_map _restrictions;
};

