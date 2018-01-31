#pragma once

#include "stdafx.h"
#include "types.h"

#include "PDigest.h"
#include "Data.h"

class NDS;
class RangePivot;

class Pivot {
 public:
  Pivot() = default;

  Pivot(uint32_t first, uint32_t second)
      : _first(first), _second(second) {
  };

  Pivot(const Pivot &other) = default;

  Pivot(Pivot &&other) = default;
  Pivot &operator=(const Pivot &other) = default;
  Pivot &operator=(Pivot &&other) = default;

#ifdef NDS_ENABLE_PAYLOAD
  inline payload_t *get_payload() {
    return _payload;
  };
  inline const payload_t &get_payload() const {
    return *_payload;
  };

  inline void set_payload(payload_t *rhs) {
    assert(_payload == nullptr);
    assert(rhs != nullptr);
    _payload = rhs;
  }
#endif // NDS_ENABLE_PAYLOAD

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

  inline void append(const Pivot &rhs) {
    // append sequence
    back(rhs.back());
  }

 protected:
  uint32_t _first, _second;

#ifdef NDS_ENABLE_PAYLOAD
  payload_t *_payload{nullptr};
#endif // NDS_ENABLE_PAYLOAD

};

struct bined_pivot_t {
 public:
  uint64_t value;
  pivot_ctn *pivots;

  inline pivot_ctn &ptr() const { return *pivots; }
};

using subset_pivot_ctn = std::vector<const bined_pivot_t *>;

struct subset_t {
  CopyOption option{DefaultCopy};
  subset_pivot_ctn container;
};

using subset_ctn = std::vector<subset_t>;
