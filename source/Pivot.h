#pragma once

#include "stdafx.h"
#include "types.h"

class BinnedPivot;

class Pivot {
 public:
  Pivot() = default;

  Pivot(uint32_t first, uint32_t second) : _first(first), _second(second) {};

  Pivot(uint32_t first, uint32_t second, const std::shared_ptr<MergingDigest> &tdigest)
      : _first(first), _second(second), _tdigest(tdigest) {};

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

  // TODO tdigest
  inline const MergingDigest& tdigest() const {
    return *_tdigest;
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

 protected:
  uint32_t _first, _second;
  std::shared_ptr<MergingDigest> _tdigest{nullptr};
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
