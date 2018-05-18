//
// Created by cicerolp on 1/15/18.
//

#pragma once

#include "types.h"
#include "Data.h"

#include "Payload.h"

#include "Query.h"

#ifdef ENABLE_PDIGEST

class NDS;
class AgrrPDigest;

class PDigest : public Payload {
 public:
  friend class AgrrPDigest;

  PDigest(const DimensionSchema &schema) : Payload(schema) {
    _buffer_in = std::make_unique<std::vector<float>>();
    _buffer_mean = std::make_unique<std::array<float, PDIGEST_ARRAY_SIZE>>();
    _buffer_weight = std::make_unique<std::array<float, PDIGEST_ARRAY_SIZE>>();
  }

  std::vector<float> get_payload(Data &data, const Pivot &pivot) override;

  inline void dispose_buffers() override {
    _buffer_in.reset();
    _buffer_mean.reset();
    _buffer_weight.reset();
  }

 private:
  inline void clear_buffer() {
    _buffer_in->clear();
  }

  template<typename T>
  static std::vector<size_t> sort_indexes(const std::vector<T> &input) {
    // initialize original index locations
    std::vector<size_t> idx(input.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    gfx::timsort(idx.begin(), idx.end(), [&input](size_t i1, size_t i2) {
      return input[i1] < input[i2];
    });

    return idx;
  }

  static inline float integratedLocation(float q) {
    return PDIGEST_COMPRESSION * (asinApproximation(2 * q - 1) + M_PI / 2) / M_PI;
  }

  static inline float integratedQ(float k) {
    return (std::sin(std::min(k, PDIGEST_COMPRESSION) * M_PI / PDIGEST_COMPRESSION - M_PI / 2) + 1) / 2;
  }

  static float asinApproximation(float x);

  inline static float eval(float model[6], float vars[6]) {
    float r = 0;
    for (int i = 0; i < 6; i++) {
      r += model[i] * vars[i];
    }
    return r;
  }

  inline static float bound(float v) {
    if (v <= 0) {
      return 0;
    } else if (v >= 1) {
      return 1;
    } else {
      return v;
    }
  }

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

  // temporary data
  std::unique_ptr<std::vector<float>> _buffer_in;
  std::unique_ptr<std::array<float, PDIGEST_ARRAY_SIZE>> _buffer_mean, _buffer_weight;
};

class AgrrPDigest : public AgrrPayload {
 public:
  virtual ~AgrrPDigest() = default;

  inline bool empty() override {
    if (_buffer_mean.size() > 0) {
      merge_buffer_data();
    }

    return _lastUsedCell == 0;
  }

  uint32_t merge(size_t payload_index, const Pivot &pivot) override;
  uint32_t merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper) override;

  inline size_t get_size() {
    if (_buffer_mean.size() > 0) {
      merge_buffer_data();
    }

    return (size_t) _lastUsedCell;
  }

  inline float get_denser_sector() {
    if (_buffer_mean.size() > 0) {
      merge_buffer_data();
    }

    // TODO optimize algorithm

    // (2 * PI) / number of sectors
    static const double increment = (2.0 * M_PI) / 36.0;

    float sector = 0.f;
    float max_inv = 0.f;
    float prev_inv = 0.f;

    for (auto curr_angle = increment; curr_angle < 2.0 * M_PI; curr_angle += increment) {
      float inv = inverse(curr_angle);

      if (inv - prev_inv > max_inv) {
        sector = curr_angle;
        max_inv = inv - prev_inv;
      }

      prev_inv = inv;
    }

    if (1 - prev_inv > max_inv) {
      sector = 2.0 * M_PI;
    }

    return sector - (increment / 2.0);
  };

  inline const std::array<float, PDIGEST_ARRAY_SIZE> &get_centroids() {
    if (_buffer_mean.size() > 0) {
      merge_buffer_data();
    }

    return _mean;
  };

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

 protected:
  void merge_buffer_data();

  // points to the first unused centroid
  uint32_t _lastUsedCell{0};

  float _min = std::numeric_limits<float>::max();
  float _max = std::numeric_limits<float>::min();

  // number of points that have been added to each merged centroid
  std::array<float, PDIGEST_ARRAY_SIZE> _weight;
  // mean of points added to each merged centroid
  std::array<float, PDIGEST_ARRAY_SIZE> _mean;

  // temporary data - avoid unnecessary memory allocations
  std::vector<float> _buffer_mean, _buffer_weight;
};

#endif // ENABLE_PDIGEST