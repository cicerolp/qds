#pragma once

#include "Dimension.h"
#include "types.h"

class Temporal : public Dimension {
  struct TemporalElement {
    bool operator!=(const TemporalElement &rhs) const {
      return el.value != rhs.el.value;
    }
    bool operator==(const TemporalElement &rhs) const {
      return el.value == rhs.el.value;
    }
    bool operator<(const temporal_t &rhs) const {
      return (temporal_t) (el.value) < rhs;
    }
    bool operator<(const TemporalElement &rhs) const {
      return el.value < rhs.el.value;
    }
    bined_pivot_t el;
  };

 public:
  Temporal(const DimensionSchema &schema);
  ~Temporal() = default;

  uint32_t build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) override;
  bool query(const Query &query, subset_ctn &subsets) const override;

  void get_schema_hint(rapidjson::Writer<rapidjson::StringBuffer> &writer) const override;

 private:
  interval_t parse_interval(const std::string &str) const;
  sequence_t parse_sequence(const std::string &str) const;

  stde::dynarray<TemporalElement> _container;
};