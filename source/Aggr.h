//
// Created by cicerolp on 11/17/17.
//

#pragma once

#include "Pivot.h"
#include "Raw.h"
#include "PDigest.h"
#include "Gaussian.h"

#include "AggrEquality.h"

using json = rapidjson::Writer<rapidjson::StringBuffer>;

class Aggr {
 public:
  Aggr() = default;
  Aggr(const Query::aggr_expr &expr, size_t __index) : _expr(expr), _payload_index(__index) {};
  virtual ~Aggr() = default;

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

  size_t _payload_index{0};
  const Query::aggr_expr _expr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrGroupBy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrGroupBy : public Aggr {
 public:
  AggrGroupBy(const Query::aggr_expr &expr, size_t __index) : Aggr(expr, __index) {};
  virtual ~AggrGroupBy() = default;

  virtual void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) = 0;

  virtual void output(json &writer) {
    writer.StartArray();
    writer.EndArray();
  };

  virtual void output(std::vector<float> &raw) {
    return;
  }

  virtual void output_one_way(uint64_t value, const pipe_ctn &pipe, json &writer) {
    writer.StartArray();
    writer.EndArray();
  };

  virtual void output_one_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw) {
    return;
  };

  virtual void output_two_way(uint64_t value, const pipe_ctn &pipe, json &writer, uint32_t threshold) {
    writer.StartArray();
    writer.EndArray();
  };

  virtual void output_two_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw, uint32_t threshold) {
    return;
  };

  virtual void equality_one_way(uint64_t value, void *payload, json &writer) {
    writer.StartArray();
    writer.EndArray();
  };

  virtual void equality_one_way(uint64_t value, void *payload, std::vector<float> &raw) {
    return;
  };

  virtual void insert_mapped_keys(std::vector<uint64_t> &keys) const = 0;

  virtual void insert_mapped_keys(std::vector<uint64_t> &keys, uint32_t threshold) const = 0;

  virtual pipe_ctn get_pipe(uint64_t value, uint32_t threshold) {
    return pipe_ctn();
  }

  virtual void *get_payload(uint64_t value) {
    return nullptr;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrSummarize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrSummarize : public Aggr {
 public:
  AggrSummarize() = default;
  AggrSummarize(const Query::aggr_expr &expr, size_t __index) : Aggr(expr, __index) {};
  virtual ~AggrSummarize() = default;

  virtual void merge(const pivot_it &it_lower, const pivot_it &it_upper) = 0;
  virtual void merge(const range_it &it_lower, const range_it &it_upper) = 0;

  virtual void output(json &writer) = 0;
  virtual void output(std::vector<float> &raw) {
    return;
  }

  virtual void output(const pipe_ctn &pipe, json &writer) {
    return;
  };
  virtual void output(const pipe_ctn &pipe, std::vector<float> &raw) {
    return;
  };

  virtual void equality(void *payload, std::vector<float> &raw) {
    return;
  }

  virtual pipe_ctn get_pipe() {
    return pipe_ctn();
  }

  virtual void *get_payload() {
    return nullptr;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrCountGroupBy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrCountGroupBy : public AggrGroupBy {
 public:
  AggrCountGroupBy(const Query::aggr_expr &expr, size_t __index) :
      AggrGroupBy(expr, __index) {}
  virtual ~AggrCountGroupBy() = default;

  void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _map[value] += (*it++).size();
    }
  }

  void output(json &writer) override {
    for (const auto &pair : _map) {
      writer.StartArray();
      write_value(pair.first, writer);
      writer.Uint(pair.second);
      writer.EndArray();
    }
  }

  void output(std::vector<float> &raw) override {
    for (const auto &pair : _map) {
      raw.emplace_back(pair.second);
    }
  }

  void insert_mapped_keys(std::vector<uint64_t> &keys) const override {
    keys.clear();
    keys.reserve(_map.size());

    for (const auto &elt: _map) {
      keys.emplace_back(elt.first);
    }
  }

  void insert_mapped_keys(std::vector<uint64_t> &keys, uint32_t threshold) const override {
    keys.clear();
    keys.reserve(_map.size());

    for (const auto &elt: _map) {
      if (elt.second >= threshold) {
        keys.emplace_back(elt.first);
      }
    }
  }

  pipe_ctn get_pipe(uint64_t value, uint32_t threshold) override {
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second >= threshold) {
      return {(float) (*it).second};
    } else {
      return pipe_ctn();
    }
  }

 protected:
  // [key] -> [count]
  std::map<uint64_t, uint32_t> _map;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrCountSummarize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrCountSummarize : public AggrSummarize {
 public:
  AggrCountSummarize() = default;
  AggrCountSummarize(const Query::aggr_expr &expr, size_t __index) :
      AggrSummarize(expr, __index) {}
  virtual ~AggrCountSummarize() = default;

  void merge(const pivot_it &it_lower, const pivot_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _count += (*it++).size();
    }
  }

  void merge(const range_it &it_lower, const range_it &it_upper) override {
    auto it = it_lower;

    while (it != it_upper) {
      _count += (*it++).pivot.size();
    }
  }

  void output(json &writer) override {
    writer.Uint(_count);
  }

  pipe_ctn get_pipe() override {
    return {(float) _count};
  }

 protected:
  uint32_t _count{0};
};

#ifdef NDS_ENABLE_PAYLOAD
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrPayloadGroupBy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class AggrPayloadGroupBy : public AggrGroupBy {
 public:
  AggrPayloadGroupBy(const Query::aggr_expr &expr, size_t __index) :
      AggrGroupBy(expr, __index) {}
  virtual ~AggrPayloadGroupBy() = default;

  void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) override {
    uint32_t count = _map[value].payload.merge(_payload_index, it_lower, it_upper);
    _map[value].count += count;
  }

  void insert_mapped_keys(std::vector<uint64_t> &keys) const override {
    keys.clear();
    keys.reserve(_map.size());

    for (const auto &elt: _map) {
      keys.emplace_back(elt.first);
    }
  }
  void insert_mapped_keys(std::vector<uint64_t> &keys, uint32_t threshold) const override {
    keys.clear();
    keys.reserve(_map.size());

    for (const auto &elt: _map) {
      if (elt.second.count >= threshold) {
        keys.emplace_back(elt.first);
      }
    }
  }

  virtual void *get_payload(uint64_t value) override {
    auto it = _map.find(value);

    if (it == _map.end()) {
      return nullptr;
    } else {
      return &(*it).second.payload;
    }
  }

 protected:
  struct payload_pair_t {
    uint32_t count{0};
    T payload;
  };

  // [key] -> [count, payload]
  std::map<uint64_t, payload_pair_t> _map;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrPayloadSummarize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class AggrPayloadSummarize : public AggrSummarize {
 public:
  AggrPayloadSummarize() = default;
  AggrPayloadSummarize(const Query::aggr_expr &expr, size_t __index) :
      AggrSummarize(expr, __index) {}
  virtual ~AggrPayloadSummarize() = default;

  void merge(const pivot_it &it_lower, const pivot_it &it_upper) override {
    _map.merge(_payload_index, it_lower, it_upper);
  }

  void merge(const range_it &it_lower, const range_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _map.merge(_payload_index, (*it++).pivot);
    }
  }

  virtual void *get_payload() override {
    return &_map;
  }

 protected:
  T _map;
};
#endif // NDS_ENABLE_PAYLOAD

#ifdef ENABLE_PDIGEST
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrPDigestGroupBy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrPDigestGroupBy : public AggrPayloadGroupBy<AgrrPDigest> {
 public:
  AggrPDigestGroupBy(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadGroupBy(expr, __index) {}
  virtual ~AggrPDigestGroupBy() = default;

  void output(json &writer) override {
    auto parameters = AgrrPDigest::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          writer.StartArray();
          write_value(pair.first, writer);
          // writer.Double(q);
          writer.Double(pair.second.payload.quantile(q));
          writer.EndArray();
        }
      }
    } else if (_expr.first == "inverse") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          writer.StartArray();
          write_value(pair.first, writer);
          // writer.Double(q);
          writer.Double(pair.second.payload.inverse(q));
          writer.EndArray();
        }
      }
    } else if (_expr.first == "sector") {
      for (auto &pair : _map) {
        writer.StartArray();
        write_value(pair.first, writer);
        writer.Double(pair.second.payload.get_denser_sector());
        writer.EndArray();
      }
    }
  }

  void output(std::vector<float> &raw) override {
    auto parameters = AgrrPDigest::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          raw.emplace_back(pair.second.payload.quantile(q));
        }
      }
    } else if (_expr.first == "inverse") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          raw.emplace_back(pair.second.payload.inverse(q));
        }
      }
    } else if (_expr.first == "sector") {
      for (auto &pair : _map) {
        raw.emplace_back(pair.second.payload.get_denser_sector());
      }
    }
  }

  void output_one_way(uint64_t value, const pipe_ctn &pipe, json &writer) override {
    if (pipe.empty()) {
      if (_expr.first == "quantile") {
        writer.StartArray();
        write_value(value, writer);
        // writer.String("NaN");
        writer.String("NaN");
        writer.EndArray();
      } else if (_expr.first == "inverse") {
        writer.StartArray();
        write_value(value, writer);
        // writer.String("NaN");
        writer.Double(-1.0);
        writer.EndArray();
      }

    } else {
      auto it = _map.find(value);

      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);

          if (it != _map.end()) {
            writer.Double((*it).second.payload.quantile(q));
          } else {
            writer.String("NaN");
          }

          writer.EndArray();
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);

          if (it != _map.end()) {
            writer.Double((*it).second.payload.inverse(q));
          } else {
            writer.Double(-1.0);
          }

          writer.EndArray();
        }
      }
    }
  }

  void output_one_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw) override {
    if (pipe.empty()) {
      if (_expr.first == "quantile") {
        raw.emplace_back(std::numeric_limits<float>::quiet_NaN());
      } else if (_expr.first == "inverse") {
        raw.emplace_back(-1.0);
      }

    } else {
      auto it = _map.find(value);

      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          if (it != _map.end()) {
            raw.emplace_back((*it).second.payload.quantile(q));
          } else {
            raw.emplace_back(std::numeric_limits<float>::quiet_NaN());
          }
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          if (it != _map.end()) {
            raw.emplace_back((*it).second.payload.inverse(q));
          } else {
            raw.emplace_back(-1.0);
          }
        }
      }
    }
  }

  void output_two_way(uint64_t value, const pipe_ctn &pipe, json &writer, uint32_t threshold) override {
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);
          writer.Double((*it).second.payload.quantile(q));
          writer.EndArray();
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);
          writer.Double((*it).second.payload.inverse(q));
          writer.EndArray();
        }
      }
    }
  }

  void output_two_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw, uint32_t threshold) override {
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          raw.emplace_back((*it).second.payload.quantile(q));
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          raw.emplace_back((*it).second.payload.inverse(q));
        }
      }
    }
  }

  void equality_one_way(uint64_t value, void *payload, json &writer) override {
    writer.StartArray();
    // TODO implement
    writer.EndArray();
  }
  void equality_one_way(uint64_t value, void *payload, std::vector<float> &raw) override {
    static const float empty = 1.0f;

    auto it = _map.find(value);
    auto lhs = &(*it).second.payload;
    auto rhs = (AgrrPDigest *) payload;

    if (rhs == nullptr) {
      // it != _map.end()
      // right_join
      raw.emplace_back(empty);
    } else if (it != _map.end()) {
      if (_expr.first == "sector") {
        auto distance = polar::distance(lhs, rhs);
        raw.emplace_back(distance);
      } else if (_expr.first == "ks") {
        auto distance = ks::distance(lhs, rhs);
        raw.emplace_back(distance);
      } else if (_expr.first == "ksw") {
        auto distance = ks::distance_w(lhs, rhs, 2.f);
        raw.emplace_back(distance);
      }
    } else {
      // it == _map.end()
      // left_join
      raw.emplace_back(empty);
    }
  }

  pipe_ctn get_pipe(uint64_t value, uint32_t threshold) override {
    pipe_ctn pipe;
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      auto parameters = AgrrPDigest::get_parameters(_expr);

      if (_expr.first == "quantile") {
        for (auto &q : parameters) {
          pipe.emplace_back((*it).second.payload.quantile(q));
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : parameters) {
          pipe.emplace_back((*it).second.payload.inverse(q));
        }
      } else if (_expr.first == "sector") {
        pipe.emplace_back((*it).second.payload.get_denser_sector());
      }
    }

    return pipe;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrPDigestSummarize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrPDigestSummarize : public AggrPayloadSummarize<AgrrPDigest> {
 public:
  AggrPDigestSummarize() = default;
  AggrPDigestSummarize(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadSummarize(expr, __index) {}
  virtual ~AggrPDigestSummarize() = default;

  void output(json &writer) override {
    output(AgrrPDigest::get_parameters(_expr), writer);
  }

  void output(const pipe_ctn &pipe, json &writer) override {
    if (_expr.first == "quantile") {
      for (auto &q : pipe) {
        writer.StartArray();
        // writer.Double(q);
        writer.Double(_map.quantile(q));
        writer.EndArray();
      }
    } else if (_expr.first == "inverse") {
      for (auto &q : pipe) {
        writer.StartArray();
        // writer.Double(q);
        writer.Double(_map.inverse(q));
        writer.EndArray();
      }
    }
  }

  void output(const pipe_ctn &pipe, std::vector<float> &raw) override {
    if (_expr.first == "quantile") {
      for (auto &q : pipe) {
        raw.emplace_back(_map.quantile(q));
      }
    } else if (_expr.first == "inverse") {
      for (auto &q : pipe) {
        raw.emplace_back(_map.inverse(q));
      }
    }
  }

  void equality(void *payload, std::vector<float> &raw) override {
    auto lhs = &_map;
    auto rhs = (AgrrPDigest *) payload;

    if (_expr.first == "sector") {
      auto distance = polar::distance(lhs, rhs);
      raw.emplace_back(distance);
    } else if (_expr.first == "ks") {
      auto distance = ks::distance(lhs, rhs);
      raw.emplace_back(distance);
    } else if (_expr.first == "ksw") {
      auto distance = ks::distance_w(lhs, rhs, 2.f);
      raw.emplace_back(distance);
    }
  }

  pipe_ctn get_pipe() override {
    pipe_ctn pipe;

    auto parameters = AgrrPDigest::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &q : parameters) {
        pipe.emplace_back(_map.quantile(q));
      }
    } else if (_expr.first == "inverse") {
      for (auto &q : parameters) {
        pipe.emplace_back(_map.inverse(q));
      }
    } else if (_expr.first == "sector") {
      pipe.emplace_back(_map.get_denser_sector());
    }

    return pipe;
  }
};
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrGaussianGroupBy
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrGaussianGroupBy : public AggrPayloadGroupBy<AggrGaussian> {
 public:
  AggrGaussianGroupBy(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadGroupBy(expr, __index) {}
  virtual ~AggrGaussianGroupBy() = default;

  void output(json &writer) override {
    if (_expr.first == "variance") {
      for (const auto &pair : _map) {
        writer.StartArray();
        write_value(pair.first, writer);
        writer.Double(pair.second.payload.variance());
        writer.EndArray();
      }

    } else if (_expr.first == "average") {
      for (const auto &pair : _map) {
        writer.StartArray();
        write_value(pair.first, writer);
        writer.Double(pair.second.payload.average());
        writer.EndArray();
      }
    }
  }

  pipe_ctn get_pipe(uint64_t value, uint32_t threshold) override {
    pipe_ctn pipe;
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      if (_expr.first == "variance") {
        pipe.emplace_back((*it).second.payload.variance());
      } else if (_expr.first == "average") {
        pipe.emplace_back((*it).second.payload.average());
      }
    }

    return pipe;
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AggrGaussianSummarize
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AggrGaussianSummarize : public AggrPayloadSummarize<AggrGaussian> {
 public:
  AggrGaussianSummarize() = default;
  AggrGaussianSummarize(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadSummarize(expr, __index) {}
  virtual ~AggrGaussianSummarize() = default;

  void output(json &writer) override {
    if (_expr.first == "variance") {
      writer.StartArray();
      writer.Double(_map.variance());
      writer.EndArray();

    } else if (_expr.first == "average") {
      writer.StartArray();
      writer.Double(_map.average());
      writer.EndArray();
    }
  }

  virtual pipe_ctn get_pipe() override {
    if (_expr.first == "variance") {
      return {_map.variance()};
    } else if (_expr.first == "average") {
      return {_map.average()};
    } else {
      return pipe_ctn();
    }
  }
};
#endif // ENABLE_GAUSSIAN

#ifdef ENABLE_RAW
class AggrRawGroupBy : public AggrPayloadGroupBy<AggrRaw> {
 public:
  AggrRawGroupBy(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadGroupBy(expr, __index) {}
  virtual ~AggrRawGroupBy() = default;

  void output(json &writer) override {
    auto parameters = AggrRaw::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          writer.StartArray();
          write_value(pair.first, writer);
          // writer.Double(q);
          writer.Double(pair.second.payload.quantile(q));
          writer.EndArray();
        }
      }

    } else if (_expr.first == "inverse") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          writer.StartArray();
          write_value(pair.first, writer);
          // writer.Double(q);
          writer.Double(pair.second.payload.inverse(q));
          writer.EndArray();
        }
      }
    }
  }

  void output(std::vector<float> &raw) override {
    auto parameters = AggrRaw::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          raw.emplace_back(pair.second.payload.quantile(q));
        }
      }

    } else if (_expr.first == "inverse") {
      for (auto &pair : _map) {
        for (auto &q : parameters) {
          raw.emplace_back(pair.second.payload.inverse(q));
        }
      }
    }
  }

  void output_one_way(uint64_t value, const pipe_ctn &pipe, json &writer) override {
    if (pipe.empty()) {
      if (_expr.first == "quantile") {
        writer.StartArray();
        write_value(value, writer);
        // writer.String("NaN");
        writer.String("NaN");
        writer.EndArray();
      } else if (_expr.first == "inverse") {
        writer.StartArray();
        write_value(value, writer);
        // writer.String("NaN");
        writer.Double(-1.0);
        writer.EndArray();
      }

    } else {
      auto it = _map.find(value);

      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);

          if (it != _map.end()) {
            writer.Double((*it).second.payload.quantile(q));
          } else {
            writer.String("NaN");
          }

          writer.EndArray();
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);

          if (it != _map.end()) {
            writer.Double((*it).second.payload.inverse(q));
          } else {
            writer.Double(-1.0);
          }

          writer.EndArray();
        }
      }
    }
  }

  void output_one_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw) override {
    if (pipe.empty()) {
      if (_expr.first == "quantile") {
        raw.emplace_back(std::numeric_limits<float>::quiet_NaN());
      } else if (_expr.first == "inverse") {
        raw.emplace_back(-1.0);
      }

    } else {
      auto it = _map.find(value);

      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          if (it != _map.end()) {
            raw.emplace_back((*it).second.payload.quantile(q));
          } else {
            raw.emplace_back(std::numeric_limits<float>::quiet_NaN());
          }
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          if (it != _map.end()) {
            raw.emplace_back((*it).second.payload.inverse(q));
          } else {
            raw.emplace_back(-1.0);
          }
        }
      }
    }
  }

  void output_two_way(uint64_t value, const pipe_ctn &pipe, json &writer, uint32_t threshold) override {
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);
          writer.Double((*it).second.payload.quantile(q));
          writer.EndArray();
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          writer.StartArray();
          write_value(value, writer);
          // writer.Double(q);
          writer.Double((*it).second.payload.inverse(q));
          writer.EndArray();
        }
      }
    }
  }

  void output_two_way(uint64_t value, const pipe_ctn &pipe, std::vector<float> &raw, uint32_t threshold) override {
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      if (_expr.first == "quantile") {
        for (auto &q : pipe) {
          raw.emplace_back((*it).second.payload.quantile(q));
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : pipe) {
          raw.emplace_back((*it).second.payload.inverse(q));
        }
      }
    }
  }

  pipe_ctn get_pipe(uint64_t value, uint32_t threshold) override {
    pipe_ctn pipe;
    auto it = _map.find(value);

    if (it != _map.end() && (*it).second.count >= threshold) {
      auto parameters = AggrRaw::get_parameters(_expr);

      if (_expr.first == "quantile") {
        for (auto &q : parameters) {
          pipe.emplace_back((*it).second.payload.quantile(q));
        }
      } else if (_expr.first == "inverse") {
        for (auto &q : parameters) {
          pipe.emplace_back((*it).second.payload.inverse(q));
        }
      }
    }

    return pipe;
  }
};

class AggrRawSummarize : public AggrPayloadSummarize<AggrRaw> {
 public:
  AggrRawSummarize() = default;
  AggrRawSummarize(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadSummarize(expr, __index) {}
  virtual ~AggrRawSummarize() = default;

  void output(json &writer) override {
    output(AggrRaw::get_parameters(_expr), writer);
  }

  void output(const pipe_ctn &pipe, json &writer) override {
    if (_expr.first == "quantile") {
      for (auto &q : pipe) {
        writer.StartArray();
        // writer.Double(q);
        writer.Double(_map.quantile(q));
        writer.EndArray();
      }
    } else if (_expr.first == "inverse") {
      for (auto &q : pipe) {
        writer.StartArray();
        // writer.Double(q);
        writer.Double(_map.inverse(q));
        writer.EndArray();
      }
    }
  }

  pipe_ctn get_pipe() override {
    pipe_ctn pipe;

    auto parameters = AggrRaw::get_parameters(_expr);

    if (_expr.first == "quantile") {
      for (auto &q : parameters) {
        pipe.emplace_back(_map.quantile(q));
      }
    } else if (_expr.first == "inverse") {
      for (auto &q : parameters) {
        pipe.emplace_back(_map.inverse(q));
      }
    }

    return pipe;
  }
};
#endif // ENABLE_RAW