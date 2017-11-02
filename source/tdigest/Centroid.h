//
// Created by cicerolp on 11/2/17.
//

#pragma once

class AtomicInteger {
  static int32_t uniqueCount = 1;

  static int32_t getAndIncrement() {
    return uniqueCount++;
  }
};

class Centroid {
 public:
  Centroid(bool record) {
    _id = AtomicInteger::getAndIncrement();
  }

  Centroid(double x) : Centroid(false) {
    start(x, 1, AtomicInteger::getAndIncrement());
  }

  Centroid(double x, int32_t w) : Centroid(false) {
    start(x, w, AtomicInteger::getAndIncrement());
  }

  Centroid(double x, int32_t w, int32_t id) : Centroid(false) {
    start(x, w, id);
  }

  Centroid(double x, int32_t id, bool record) : Centroid(record) {
    start(x, 1, id);
  }

  Centroid(double x, int32_t w, const std::vector<double> &data) : Centroid(x, w) {
    _actualData = data;
  }

  inline void add(double x, int32_t w) {
    // TODO test actualData != null
    _actualData.emplace_back(x);

    _count += w;
    _centroid += w * (x - _centroid) / _count;
  }

  inline double mean() const {
    return _centroid;
  }

  inline int32_t count() const {
    return _count;
  }

  inline int32_t id() const {
    return _id;
  }

  inline std::vector<double> data() const {
    return _actualData;
  }

  inline void insertData(double x) {
    // TODO test actualData != null
    _actualData.emplace_back(x);
  }

  void add(double x, int32_t w, const std::vector<double> &data) {
    // TODO test actualData != null

    if (data.size() != 0) {
      for (const auto &old : data) {
        _actualData.emplace_back(old);
      }
    } else {
      _actualData.emplace_back(x);
    }

    // TODO centroid = AbstractTDigest.weightedAverage(centroid, count, x, w);
    _count += w;
  }

  static Centroid createWeighted(double x, int32_t w, const std::vector<double> &data) {
    // TODO test data != null
    Centroid r(data.size() != 0);
    r.add(x, w, data);
    return r;
  }

 private:
  double _centroid = 0;
  int32_t _count = 0;
  int32_t _id;

  // TODO dynamic allocation
  std::vector<double> _actualData;
};
