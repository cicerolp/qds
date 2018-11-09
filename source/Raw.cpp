//
// Created by cicerolp on 6/13/18.
//

#include "stdafx.h"
#include "Raw.h"
#include "Pivot.h"
#include "NDS.h"

#ifdef ENABLE_RAW

std::vector<float> Raw::get_payload(Data &data, const Pivot &pivot) {
  // sum, sum_square
  std::vector<float> payload;
  payload.reserve(pivot.size());

  for (auto p = pivot.front(); p < pivot.back(); ++p) {
    auto elt = data.payload(_schema.offset, p);

    payload.emplace_back(elt);
  }

  return payload;
}

uint32_t AggrRaw::merge(size_t payload_index, const Pivot &pivot) {
  uint32_t count = pivot.size();
  const auto &payload = pivot.get_payload(payload_index);

  for (auto ptr = payload.lower; ptr < payload.upper; ++ptr) {
    _payload.emplace_back(*ptr);
  }

  _sorted = false;

  return count;
}

uint32_t AggrRaw::merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper) {
  uint32_t count = 0;
  auto it = it_lower;

  // insert payload data
  while (it != it_upper) {
    count += (*it).size();

    auto &payload = (*it).get_payload(payload_index);

    for (auto ptr = payload.lower; ptr < payload.upper; ++ptr) {
      _payload.emplace_back(*ptr);
    }

    ++it;
  }

  _sorted = false;

  return count;
}

float AggrRaw::quantile(float q) {
  /*assert(q >= 0.0 && q <= 1.0);

  if (_payload.size() == 0) {
    // no data, no way to get a quantile
    return std::numeric_limits<float>::quiet_NaN();
  }

  sort_data();

  const double n  = _payload.size();
  const double id = (n - 1) * q;
  const double lo = std::floor(id);
  const double hi = std::ceil(id);
  const double qs = _payload[lo];
  const double h  = (id - lo);

  return (1.0 - h) * qs + h * (double)_payload[hi];*/

  assert(q >= 0.0 && q <= 1.0);

  sort_data();

  size_t n = _payload.size();

  // if values were stored in a sorted array, index would be the offset we are interested in
  const double index = n * q;

  // at the boundaries, we return min or max
  if (index < 0.5) {
    return (double)_payload[0] * index;
  }

  // in between we interpolate between centroids
  double weightSoFar = 0.5;

  for (auto i = 0; i < _payload.size() - 1; ++i) {
    double dw = 1;

    if (weightSoFar + dw > index) {
      // centroids i and i+1 bracket our current point
      double z1 = index - weightSoFar;
      double z2 = weightSoFar + dw - index;
      return weightedAverage(_payload[i], z2, _payload[i + 1], z1);
    }

    weightSoFar += dw;
  }

  return _payload[n - 1] + (_payload[n - 1] * (index - n));
}

float AggrRaw::inverse(float value) {
  return 0.f;
}

void AggrRaw::sort_data() {
  if (_sorted) return;

  std::sort(_payload.begin(), _payload.end());

  PRINT_TIMER("size", _payload.size(), "sizeof", sizeof(float), "buffer")

  _sorted = true;
}

#endif // ENABLE_RAW