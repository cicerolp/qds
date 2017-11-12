#pragma once

#include "stdafx.h"
#include "types.h"

class BinnedPivot;

class Pivot {
 public:
  Pivot() = default;

  Pivot(uint32_t first, uint32_t second, bool readPDigestData = true) : _first(first), _second(second) {
    if (readPDigestData) {
      // TODO get value from dataset
      std::vector<double> inMean;
      inMean.reserve(second - first);

      for (auto p = first; p < second; ++p) {
        inMean.emplace_back(rand() % 1001);
      }
      std::vector<double> inWeight(second - first, 1);

      add(inMean, inWeight);
    }
  };

  Pivot(const Pivot &other) = default;
  Pivot(Pivot &&other) = default;
  Pivot &operator=(const Pivot &other) = default;
  Pivot &operator=(Pivot &&other) = default;

  inline bool empty() const { return (back() - front()) == 0; }
  inline uint32_t size() const { return back() - front(); }

  inline uint32_t front() const { return _first; }
  inline uint32_t back() const { return _second; }

  inline void front(uint32_t value) { _first = value; }
  inline void back(uint32_t value) { _second = value; }

  inline bool operator==(const Pivot &other) const {
    return front() == other.front() && back() == other.back();
  }
  inline bool operator<(const Pivot &other) const {
    return front() < other.front();
  }

  inline bool ends_before(const Pivot &other) const {
    return back() <= other.front();
  }
  inline bool begins_after(const Pivot &other) const {
    return front() >= other.back();
  }

  friend std::ostream &operator<<(std::ostream &stream, const Pivot &pivot) {
    stream << "[" << pivot.front() << "," << pivot.back() << "]";
    return stream;
  }

  static inline bool is_sequence(const Pivot &lhs, const Pivot &rhs) {
    return lhs.back() == rhs.front();
  }
  static inline bool lower_bound_comp(const Pivot &lhs, const Pivot &rhs) {
    return lhs.front() < rhs.front();
  }
  static inline bool upper_bound_comp(const Pivot &lhs, const Pivot &rhs) {
    return lhs.back() <= rhs.front();
  }

  void merge_pivot(const Pivot &rhs);

  // quantiles
  void merge_pdigest(const Pivot &other);

  double quantile(double q) const;

 protected:
  uint32_t _first, _second;

  // quantiles
  // points to the first unused centroid
  uint32_t _lastUsedCell{0};

  double _min = std::numeric_limits<double>::max();
  double _max = std::numeric_limits<double>::min();

  // number of points that have been added to each merged centroid
  std::array<double, PDIGEST_ARRAY_SIZE> _weight;
  // mean of points added to each merged centroid
  std::array<double, PDIGEST_ARRAY_SIZE> _mean;

 private:
  void add(std::vector<double> inMean, std::vector<double> inWeight);

  inline double integratedLocation(double q) const {
    return PDIGEST_COMPRESSION * (asinApproximation(2 * q - 1) + M_PI / 2) / M_PI;
  }

  inline double integratedQ(double k) const {
    return (std::sin(std::min(k, PDIGEST_COMPRESSION) * M_PI / PDIGEST_COMPRESSION - M_PI / 2) + 1) / 2;
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
};

using pivot_ctn = stde::dynarray<Pivot>;
using pivot_it = pivot_ctn::const_iterator;

using link_ctn = std::vector<pivot_ctn *>;
using link_it = link_ctn::const_iterator;

using build_ctn = std::vector<Pivot>;
using build_it = build_ctn::const_iterator;

struct binned_t {
 public:
  // shared (false) or proper (true) content
  bool proper{false};

  uint64_t value;
  pivot_ctn *pivots;

  ~binned_t() {
    if (proper) delete pivots;
  }
  inline pivot_ctn &ptr() const { return *pivots; }
};

using binned_ctn = std::vector<const binned_t *>;
using binned_it = binned_ctn::const_iterator;

struct subset_t {
  subset_t() : option(DefaultCopy) {}
  CopyOption option;
  binned_ctn container;
};

using subset_container = std::vector<subset_t>;
