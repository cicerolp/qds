//
// Created by cicerolp on 6/13/18.
//

#pragma once

#include "Query.h"

#include "Data.h"
#include "Payload.h"

#ifdef ENABLE_RAW

class Raw : public Payload {
 public:
  Raw(const DimensionSchema &schema) : Payload(schema) {}

  std::vector<float> get_payload(Data &data, const Pivot &pivot) override;
};

class AggrRaw : public AgrrPayload {
 public:
  virtual ~AggrRaw() = default;

  inline bool empty() const override {
    return _payload.size() == 0;
  }

  uint32_t merge(size_t payload_index, const Pivot &pivot) override;
  uint32_t merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper) override;

  float quantile(float q);
  float inverse(float value);

  static inline pipe_ctn get_parameters(const Query::aggr_expr &expr) {
    auto clausule = boost::trim_copy_if(expr.second.second, boost::is_any_of("()"));

    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    pipe_ctn pipe;
    for (auto &q : tokens) {
      pipe.emplace_back(std::stof(q));
    }

    return pipe;
  }

 private:
  void sort_data();

  inline static float weightedAverage(float x1, float w1, float x2, float w2) {
    if (x1 <= x2) {
      return weightedAverageSorted(x1, w1, x2, w2);
    } else {
      return weightedAverageSorted(x2, w2, x1, w1);
    }
  }

  inline static float weightedAverageSorted(float x1, float w1, float x2, float w2) {
    const float x = (x1 * w1 + x2 * w2) / (w1 + w2);
    return std::max(x1, std::min(x, x2));
  }

  bool _sorted{false};
  std::vector<float> _payload;
};

#endif // ENABLE_RAW