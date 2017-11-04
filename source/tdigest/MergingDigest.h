//
// Created by cicerolp on 11/4/17.
//

#pragma once

class MergingDigest {
 public:
  MergingDigest(double compression) : MergingDigest(compression, -1) {}

  MergingDigest(double compression, int32_t size);

  void add(std::vector<double> inMean, std::vector<double> inWeight);

 private:
  inline double integratedLocation(double q) const {
    return _compression * (asinApproximation(2 * q - 1) + M_PI / 2) / M_PI;
  }

  inline double integratedQ(double k) const {
    return (std::sin(std::min(k, _compression) * M_PI / _compression - M_PI / 2) + 1) / 2;
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

  double _min = std::numeric_limits<double>::max();
  double _max = std::numeric_limits<double>::min();

  const double _compression;

  // points to the first unused centroid
  int32_t _lastUsedCell;

  // sum_i weight[i]  See also unmergedWeight
  double _totalWeight = 0;

  // number of points that have been added to each merged centroid
  std::vector<double> _weight;
  // mean of points added to each merged centroid
  std::vector<double> _mean;

  // history of all data added to centroids (for testing purposes)
  // std::vector<std::vector<double>> _data;

  // sum_i tempWeight[i]
  //double unmergedWeight = 0;

  // this is the index of the next temporary centroid
  // this is a more Java-like convention than lastUsedCell uses
  //int32_t _tempUsed = 0;
  //std::vector<double> _tempWeight;
  //std::vector<double> _tempMean;
  //std::vector<std::vector<double>> _tempData;

  // array used for sorting the temp centroids.  This is a field
  // to avoid allocations during operation
  //std::vector<int32_t> _order;

  static const bool usePieceWiseApproximation = true;
  static const bool useWeightLimit = true;
};
