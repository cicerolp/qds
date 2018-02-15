//
// Created by cicerolp on 11/17/17.
//

#pragma once

#include "Pivot.h"
#include "PDigest.h"
#include "Gaussian.h"

using json = rapidjson::Writer<rapidjson::StringBuffer>;

class Aggr {
 public:
  Aggr() = default;
  Aggr(const Query::aggr_expr &expr, size_t __index) : _expr(expr), _payload_index(__index) {};

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

/////////////////////////////////////////////////////////////////////////////////////////
// Subset
/////////////////////////////////////////////////////////////////////////////////////////

class AggrSubset : public Aggr {
 public:
  AggrSubset(const Query::aggr_expr &expr, size_t __index) : Aggr(expr, __index) {};

  virtual void merge(size_t el, const pivot_it &it_lower, const pivot_it &it_upper) = 0;
  virtual void output(size_t el, uint64_t value, json &writer) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Range
/////////////////////////////////////////////////////////////////////////////////////////

class AggrRange : public Aggr {
 public:
  AggrRange(const Query::aggr_expr &expr, size_t __index) : Aggr(expr, __index) {};

  virtual void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) = 0;
  virtual void output(json &writer) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////
// None
/////////////////////////////////////////////////////////////////////////////////////////

class AggrNone : public Aggr {
 public:
  AggrNone(const Query::aggr_expr &expr, size_t __index) : Aggr(expr, __index) {};

  virtual void merge(const pivot_it &it_lower, const pivot_it &it_upper) = 0;
  virtual void merge(const range_it &it_lower, const range_it &it_upper) = 0;
  virtual void output(json &writer) = 0;
};

using aggrs_subset_t = std::vector<std::unique_ptr<AggrSubset>>;
using aggrs_range_t = std::vector<std::unique_ptr<AggrRange>>;
using aggrs_none_t = std::vector<std::unique_ptr<AggrNone>>;

// count
/////////////////////////////////////////////////////////////////////////////////////////

class AggrCountSubset : public AggrSubset {
 public:
  AggrCountSubset(const Query::aggr_expr &expr, size_t __index, size_t size) :
      AggrSubset(expr, __index), _map(size, 0) {}

  virtual void merge(size_t el, const pivot_it &it_lower, const pivot_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _map[el] += (*it++).size();
    }
  }

  virtual void output(size_t el, uint64_t value, json &writer) override {
    if (_map[el] == 0) return;

    writer.StartArray();
    write_value(value, writer);
    writer.Uint(_map[el]);
    writer.EndArray();
  }

 protected:
  std::vector<uint32_t> _map;
};

// count
/////////////////////////////////////////////////////////////////////////////////////////
class AggrCountRange : public AggrRange {
 public:
  AggrCountRange(const Query::aggr_expr &expr, size_t __index) :
      AggrRange(expr, __index) {}

  virtual void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _map[value] += (*it++).size();
    }
  }

  virtual void output(json &writer) override {
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

// count
/////////////////////////////////////////////////////////////////////////////////////////
class AggrCountNone : public AggrNone {
 public:
  AggrCountNone(const Query::aggr_expr &expr, size_t __index) :
      AggrNone(expr, __index) {}

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
 protected:
  uint32_t _count{0};
};

#ifdef NDS_ENABLE_PAYLOAD
template<typename T>
class AggrPayloadSubset : public AggrSubset {
 public:
  AggrPayloadSubset(const Query::aggr_expr &expr, size_t __index, size_t size) :
      AggrSubset(expr, __index), _map(size) {}

  void merge(size_t el, const pivot_it &it_lower, const pivot_it &it_upper) override {
    _map[el].merge(_payload_index, it_lower, it_upper);
  }

 protected:
  std::vector<T> _map;
};

template<typename T>
class AggrPayloadRange : public AggrRange {
 public:
  AggrPayloadRange(const Query::aggr_expr &expr, size_t __index) :
      AggrRange(expr, __index) {}

  void merge(uint64_t value, const pivot_it &it_lower, const pivot_it &it_upper) override {
    _map[value].merge(_payload_index, it_lower, it_upper);
  }

 protected:
  std::map<uint64_t, T> _map;
};

template<typename T>
class AggrPayloadNone : public AggrNone {
 public:
  AggrPayloadNone(const Query::aggr_expr &expr, size_t __index) :
      AggrNone(expr, __index) {}

  void merge(const pivot_it &it_lower, const pivot_it &it_upper) override {
    _map.merge(_payload_index, it_lower, it_upper);
  }

  void merge(const range_it &it_lower, const range_it &it_upper) override {
    auto it = it_lower;
    while (it != it_upper) {
      _map.merge(_payload_index, (*it++).pivot);
    }
  }

 protected:
  T _map;
};
#endif // NDS_ENABLE_PAYLOAD

#ifdef ENABLE_PDIGEST
class AggrPDigestSubset : public AggrPayloadSubset<AgrrPDigest> {
 public:
  AggrPDigestSubset(const Query::aggr_expr &expr, size_t __index, size_t size) :
      AggrPayloadSubset(expr, __index, size) {}

  void output(size_t el, uint64_t value, json &writer) override {
    if (_map[el].empty()) return;

    auto clausule = boost::trim_copy_if(_expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    if (_expr.first == "quantile") {
      for (auto &q : tokens) {
        auto parameter = std::stof(q);

        writer.StartArray();
        write_value(value, writer);
        writer.Double(parameter);
        writer.Double(_map[el].quantile(parameter));
        writer.EndArray();
      }

    } else if (_expr.first == "inverse") {
      for (auto &q : tokens) {
        auto parameter = std::stof(q);

        writer.StartArray();
        write_value(value, writer);
        writer.Double(parameter);
        writer.Double(_map[el].inverse(parameter));
        writer.EndArray();
      }
    }
  }
};

class AggrPDigestRange : public AggrPayloadRange<AgrrPDigest> {
 public:
  AggrPDigestRange(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadRange(expr, __index) {}

  void output(json &writer) override {
    auto clausule = boost::trim_copy_if(_expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    if (_expr.first == "quantile") {
      for (const auto &pair : _map) {
        for (auto &q : tokens) {
          auto parameter = std::stof(q);

          writer.StartArray();
          write_value(pair.first, writer);
          writer.Double(parameter);
          writer.Double(pair.second.quantile(parameter));
          writer.EndArray();
        }
      }

    } else if (_expr.first == "inverse") {
      for (const auto &pair : _map) {
        for (auto &q : tokens) {
          auto parameter = std::stof(q);

          writer.StartArray();
          write_value(pair.first, writer);
          writer.Double(parameter);
          writer.Double(pair.second.inverse(parameter));
          writer.EndArray();
        }
      }
    }
  }
};

class AggrPDigestNone : public AggrPayloadNone<AgrrPDigest> {
 public:
  AggrPDigestNone(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadNone(expr, __index) {}

  void output(json &writer) override {
    auto clausule = boost::trim_copy_if(_expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    if (_expr.first == "quantile") {
      for (auto &q : tokens) {
        auto parameter = std::stof(q);

        writer.StartArray();
        writer.Double(parameter);
        writer.Double(_map.quantile(parameter));
        writer.EndArray();
      }

    } else if (_expr.first == "inverse") {
      for (auto &q : tokens) {
        auto parameter = std::stof(q);

        writer.StartArray();
        writer.Double(parameter);
        writer.Double(_map.inverse(parameter));
        writer.EndArray();
      }
    }
  }
};
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
class AggrGaussianSubset : public AggrPayloadSubset<AggrGaussian> {
 public:
  AggrGaussianSubset(const Query::aggr_expr &expr, size_t __index, size_t size) :
      AggrPayloadSubset(expr, __index, size) {}

  void output(size_t el, uint64_t value, json &writer) override {
    if (_map[el].empty()) return;

    if (_expr.first == "variance") {
      writer.StartArray();
      write_value(value, writer);
      writer.Double(_map[el].variance());
      writer.EndArray();

    } else if (_expr.first == "average") {
      writer.StartArray();
      write_value(value, writer);
      writer.Double(_map[el].average());
      writer.EndArray();
    }
  }
};

class AggrGaussianRange : public AggrPayloadRange<AggrGaussian> {
 public:
  AggrGaussianRange(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadRange(expr, __index) {}

  void output(json &writer) override {
    auto clausule = boost::trim_copy_if(_expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    if (_expr.first == "variance") {
      for (const auto &pair : _map) {
        writer.StartArray();
        write_value(pair.first, writer);
        writer.Double(pair.second.variance());
        writer.EndArray();
      }

    } else if (_expr.first == "average") {
      for (const auto &pair : _map) {
        writer.StartArray();
        write_value(pair.first, writer);
        writer.Double(pair.second.average());
        writer.EndArray();
      }
    }
  }
};

class AggrGaussianNone : public AggrPayloadNone<AggrGaussian> {
 public:
  AggrGaussianNone(const Query::aggr_expr &expr, size_t __index) :
      AggrPayloadNone(expr, __index) {}

  void output(json &writer) override {
    auto clausule = boost::trim_copy_if(_expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

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
};
#endif // ENABLE_GAUSSIAN
