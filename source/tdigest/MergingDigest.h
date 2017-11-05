//
// Created by cicerolp on 11/4/17.
//

#pragma once

class NDS;

class MergingDigest {
 public:
  MergingDigest() {
    allocateArrays(-1);
  }

  MergingDigest(NDS &nds, uint32_t front, uint32_t back) {
    allocateArrays(-1);

    add(nds, front, back);
  }

  void add(NDS &nds, uint32_t front, uint32_t back);

  void add(std::vector<double> inMean, std::vector<double> inWeight);

  void merge(const MergingDigest &other);

  double quantile(double q);

  inline int32_t centroidCount() const {
    return _mean.size();
  }

  inline void shrink_to_fit() {
    _mean.shrink_to_fit();
    _weight.shrink_to_fit();
  }

 private:
  inline double integratedLocation(double q) const {
    return TDIGEST_COMPRESSION * (asinApproximation(2 * q - 1) + M_PI / 2) / M_PI;
  }

  inline double integratedQ(double k) const {
    return (std::sin(std::min(k, TDIGEST_COMPRESSION) * M_PI / TDIGEST_COMPRESSION - M_PI / 2) + 1) / 2;
  }

  static double asinApproximation(double x);

  inline static double eval(double model[6], double vars[6]) {
    double r = 0;
    for (int i = 0; i < 6; i++) {
      r += model[i] * vars[i];
    }
    return r;
  }

  inline static double bound(double v) {
    if (v <= 0) {
      return 0;
    } else if (v >= 1) {
      return 1;
    } else {
      return v;
    }
  }

  inline static double weightedAverage(double x1, double w1, double x2, double w2) {
    if (x1 <= x2) {
      return weightedAverageSorted(x1, w1, x2, w2);
    } else {
      return weightedAverageSorted(x2, w2, x1, w1);
    }
  }

  inline static double weightedAverageSorted(double x1, double w1, double x2, double w2) {
    const double x = (x1 * w1 + x2 * w2) / (w1 + w2);
    return std::max(x1, std::min(x, x2));
  }

  template<typename T>
  std::vector<size_t> sort_indexes(const std::vector<T> &v) {

    // initialize original index locations
    std::vector<size_t> idx(v.size());
    std::iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    std::sort(idx.begin(), idx.end(),
              [&v](size_t i1, size_t i2) { return v[i1] < v[i2]; });

    return idx;
  }

  inline void allocateArrays(int32_t size) {
    if (size == -1) {
      size = (int32_t) (2 * std::ceil(TDIGEST_COMPRESSION));
      if (useWeightLimit) {
        // the weight limit approach generates smaller centroids than necessary
        // that can result in using a bit more memory than expected
        size += 10;
      }
    }

    _weight.reserve(size);
    _mean.reserve(size);
  }

  double _min = std::numeric_limits<double>::max();
  double _max = std::numeric_limits<double>::min();

  // number of points that have been added to each merged centroid
  std::vector<double> _weight;
  // mean of points added to each merged centroid
  std::vector<double> _mean;

  static const bool usePieceWiseApproximation = true;
  static const bool useWeightLimit = false;
};
