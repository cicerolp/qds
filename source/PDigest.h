//
// Created by cicerolp on 1/15/18.
//

#pragma once

#include "types.h"

#ifdef ENABLE_PDIGEST

class NDS;

class PDigest {
 public:
  void merge(const Pivot &pivot);
  void merge(pivot_it &it_lower, pivot_it &it_upper);

  float quantile(float q) const;
  float inverse(float value) const;

  static payload_t *get_payload(uint32_t first, uint32_t second, NDS &nds);

 private:
  void merge_buffer_data();

  template<typename T>
  static std::vector<size_t> sort_indexes(const std::vector<T> &v) {

    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
              [&v](size_t i1, size_t i2) { return v[i1] < v[i2]; });

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

#endif