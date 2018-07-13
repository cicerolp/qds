//
// Created by cicerolp on 1/15/18.
//

#include "stdafx.h"
#include "PDigest.h"
#include "Pivot.h"
#include "NDS.h"

#ifdef ENABLE_PDIGEST

std::vector<float> PDigest::get_payload(Data &data, const Pivot &pivot) {
  clear_buffer();

  // copy payload data
  _buffer_in->reserve(pivot.back() - pivot.front());

  for (auto p = pivot.front(); p < pivot.back(); ++p) {
    _buffer_in->emplace_back(data.payload(_schema.offset, p));
  }

  // temporary data
  uint32_t lastUsedCell{0};

  int32_t incomingCount = _buffer_in->size();

  // sort input
  gfx::timsort(_buffer_in->begin(), _buffer_in->end());

  float totalWeight = _buffer_in->size();

  lastUsedCell = 0;
  (*_buffer_mean)[lastUsedCell] = (*_buffer_in)[0];
  (*_buffer_weight)[lastUsedCell] = 1;

  float wSoFar = 0;
  float k1 = 0;
  // weight will contain all zeros
  float wLimit = totalWeight * integratedQ(k1 + 1);

  bool weight_equals_one = true;

  for (int i = 1; i < incomingCount; ++i) {
    float proposedWeight = (*_buffer_weight)[lastUsedCell] + 1;

    float projectedW = wSoFar + proposedWeight;

    if (projectedW <= wLimit) {
      weight_equals_one = false;

      // next point will fit
      // so merge into existing centroidx
      (*_buffer_weight)[lastUsedCell] += 1;
      (*_buffer_mean)[lastUsedCell] = (*_buffer_mean)[lastUsedCell]
          + ((*_buffer_in)[i] - (*_buffer_mean)[lastUsedCell]) / (*_buffer_weight)[lastUsedCell];

    } else {
      // didn't fit ... move to next output, copy out first centroid
      wSoFar += (*_buffer_weight)[lastUsedCell];

      k1 = integratedLocation(wSoFar / totalWeight);
      wLimit = totalWeight * integratedQ(k1 + 1);

      lastUsedCell++;
      (*_buffer_mean)[lastUsedCell] = (*_buffer_in)[i];
      (*_buffer_weight)[lastUsedCell] = 1;
    }
  }

  // points to next empty cell
  lastUsedCell++;

  std::vector<float> payload;

#ifdef PDIGEST_OPTIMIZE_ARRAY
  if (weight_equals_one) {
    payload.reserve(lastUsedCell + 1);

    // insert p-digest data
    payload.insert(payload.end(), _buffer_mean->begin(), _buffer_mean->begin() + lastUsedCell);

    // payload does not contains weight array
    payload.emplace_back(0.f);

    assert(payload.size() == (lastUsedCell + 1));

  } else {
    payload.reserve(lastUsedCell * 2 + 1);

    // insert p-digest data
    payload.insert(payload.end(), _buffer_mean->begin(), _buffer_mean->begin() + lastUsedCell);
    payload.insert(payload.end(), _buffer_weight->begin(), _buffer_weight->begin() + lastUsedCell);

    // payload contains weight array
    payload.emplace_back(1.f);

    assert(payload.size() == (lastUsedCell * 2 + 1));
  }
#else
  payload.reserve(lastUsedCell * 2);

  payload.insert(payload.end(), _buffer_mean->.begin(), _buffer_mean->.begin() + lastUsedCell);
  payload.insert(payload.end(), _buffer_weight->.begin(), _buffer_weight->.begin() + lastUsedCell);
#endif // PDIGEST_OPTIMIZE_ARRAY

  return payload;
}

float PDigest::asinApproximation(float x) {
#ifdef PDIGEST_PIECE_WISE_APPROXIMATION
  if (x < 0) {
    return -asinApproximation(-x);
  } else {
    // this approximation works by breaking that range from 0 to 1 into 5 regions
    // for all but the region nearest 1, rational polynomial models get us a very
    // good approximation of asin and by interpolating as we move from region to
    // region, we can guarantee continuity and we happen to get monotonicity as well.
    // for the values near 1, we just use Math.asin as our region "approximation".

    // cutoffs for models. Note that the ranges overlap. In the overlap we do
    // linear interpolation to guarantee the overall result is "nice"
    float c0High = 0.1;
    float c1High = 0.55;
    float c2Low = 0.5;
    float c2High = 0.8;
    float c3Low = 0.75;
    float c3High = 0.9;
    float c4Low = 0.87;
    if (x > c3High) {
      return std::asin(x);
    } else {
      // the models
      float m0[] = {0.2955302411, 1.2221903614, 0.1488583743, 0.2422015816, -0.3688700895, 0.0733398445};
      float m1[] = {-0.0430991920, 0.9594035750, -0.0362312299, 0.1204623351, 0.0457029620, -0.0026025285};
      float
          m2[] = {-0.034873933724, 1.054796752703, -0.194127063385, 0.283963735636, 0.023800124916, -0.000872727381};
      float m3[] = {-0.37588391875, 2.61991859025, -2.48835406886, 1.48605387425, 0.00857627492, -0.00015802871};

      // the parameters for all of the models
      float vars[] = {1, x, x * x, x * x * x, 1 / (1 - x), 1 / (1 - x) / (1 - x)};

      // raw grist for interpolation coefficients
      float x0 = bound((c0High - x) / c0High);
      float x1 = bound((c1High - x) / (c1High - c2Low));
      float x2 = bound((c2High - x) / (c2High - c3Low));
      float x3 = bound((c3High - x) / (c3High - c4Low));

      // interpolation coefficients
      //noinspection UnnecessaryLocalVariable
      float mix0 = x0;
      float mix1 = (1 - x0) * x1;
      float mix2 = (1 - x1) * x2;
      float mix3 = (1 - x2) * x3;
      float mix4 = 1 - x3;

      // now mix all the results together, avoiding extra evaluations
      float r = 0;
      if (mix0 > 0) {
        r += mix0 * eval(m0, vars);
      }
      if (mix1 > 0) {
        r += mix1 * eval(m1, vars);
      }
      if (mix2 > 0) {
        r += mix2 * eval(m2, vars);
      }
      if (mix3 > 0) {
        r += mix3 * eval(m3, vars);
      }
      if (mix4 > 0) {
        // model 4 is just the real deal
        r += mix4 * std::asin(x);
      }
      return r;
    }
  }
#else
  return std::asin(x);
#endif
}

uint32_t AgrrPDigest::merge(size_t payload_index, const Pivot &pivot) {
  uint32_t count = pivot.size();

#ifdef PDIGEST_OPTIMIZE_ARRAY
  const auto &payload = pivot.get_payload(payload_index);
  uint32_t payload_size = payload.upper - payload.lower - 1;

  if (payload.lower[payload_size] == 0.f) {
    // payload does not contains weight array

    for (auto ptr = payload.lower; ptr < payload.lower + payload_size; ++ptr) {
      // mean, weight
      _buffer.emplace_back(*ptr, 1.f);
    }
  } else {
    // payload contains weight array
    _buffer.insert(_buffer.end(), &((centroid *) payload.lower)[0], &((centroid *) payload.lower)[payload_size]);
  }
#else
  const auto &payload = pivot.get_payload(payload_index);
  uint32_t payload_size = payload.upper - payload.lower - 1;

  _buffer.insert(_buffer.end(), &((centroid *) payload.lower)[0], &((centroid *) payload.lower)[payload_size]);
#endif // PDIGEST_OPTIMIZE_ARRAY

  if (_buffer.size() >= PDIGEST_BUFFER_SIZE) {
    merge_buffer_data();
  }

  return count;
}

uint32_t AgrrPDigest::merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper) {
  uint32_t count = 0;

#ifdef PDIGEST_OPTIMIZE_ARRAY
  auto it = it_lower;
  // insert payload data
  while (it != it_upper) {
    count += (*it).size();

    auto &payload = (*it).get_payload(payload_index);
    uint32_t payload_size = payload.upper - payload.lower - 1;

    if (payload.lower[payload_size] == 0.f) {
      // payload does not contains weight array

      for (auto ptr = payload.lower; ptr < payload.lower + payload_size; ++ptr) {
        // mean, weight
        _buffer.emplace_back(*ptr, 1.f);
      }
    } else {
      // payload contains weight array
      _buffer.insert(_buffer.end(), &((centroid *) payload.lower)[0], &((centroid *) payload.lower)[payload_size]);
    }

    if (_buffer.size() >= PDIGEST_BUFFER_SIZE) {
      merge_buffer_data();
    }

    ++it;
  }
#else
  auto it = it_lower;
  // insert payload data
  while (it != it_upper) {
    count += (*it).size();

    auto &payload = (*it).get_payload(payload_index);
    uint32_t payload_size = payload.upper - payload.lower - 1;

    _buffer.insert(_buffer.end(), &((centroid *) payload.lower)[0], &((centroid *) payload.lower)[payload_size]);

    if (_buffer.size() >= PDIGEST_BUFFER_SIZE) {
      merge_buffer_data();
    }
    ++it;
  }
#endif // PDIGEST_OPTIMIZE_ARRAY

  return count;
}

float AgrrPDigest::quantile(float q) {
  if (_buffer.size() > 0) {
    merge_buffer_data();
  }

  if (_lastUsedCell == 0) {
    // no centroids means no data, no way to get a quantile
    return std::numeric_limits<float>::quiet_NaN();
  }

  // we know that there are at least two centroids now
  int32_t n = _lastUsedCell;

  float totalWeight = 0.f;
  for (auto i = 0; i < _lastUsedCell; ++i) {
    totalWeight += _centroids[i].weight;
  }

  // if values were stored in a sorted array, index would be the offset we are interested in
  const float index = q * totalWeight;

  // at the boundaries, we return min or max
  if (index < _centroids[0].weight / 2) {
    return _min + 2 * index / _centroids[0].weight * (_centroids[0].mean - _min);
  }

  // in between we interpolate between centroids
  float weightSoFar = _centroids[0].weight / 2;

  for (auto i = 0; i < n - 1; ++i) {
    float dw = (_centroids[i].weight + _centroids[i + 1].weight) / 2;

    if (weightSoFar + dw > index) {
      // centroids i and i+1 bracket our current point
      float z1 = index - weightSoFar;
      float z2 = weightSoFar + dw - index;
      return PDigest::weightedAverage(_centroids[i].mean, z2, _centroids[i + 1].mean, z1);
    }

    weightSoFar += dw;
  }

  // weightSoFar = totalWeight - weight[n-1]/2 (very nearly)
  // so we interpolate out to max value ever seen
  float z1 = index - totalWeight - _centroids[n - 1].weight / 2.0;
  float z2 = _centroids[n - 1].weight / 2 - z1;

  return PDigest::weightedAverage(_centroids[n - 1].mean, z1, _max, z2);
}

float AgrrPDigest::inverse(float value) {
  if (_buffer.size() > 0) {
    merge_buffer_data();
  }

  if (_lastUsedCell == 0) {
    // no centroids means no data, no way to get a quantile
    return std::numeric_limits<float>::quiet_NaN();
  }

  auto it = std::lower_bound(_centroids.begin(), _centroids.begin() + _lastUsedCell, value,
                             [](const auto &lhs, const auto &rhs) {
                               return lhs.mean < rhs;
                             });

  // it == end of data
  if (it == (_centroids.begin() + _lastUsedCell)) {
    return 1.0f;
  }

  // it == begin of data
  if (it == _centroids.begin()) {
    return 0.f;
  }

  auto index = it - _centroids.begin();

  // in between we interpolate between centroids
  float weightSoFar = _centroids[0].weight / 2;

  for (auto i = 0; i < index - 1; ++i) {
    weightSoFar += (_centroids[i].weight + _centroids[i + 1].weight) / 2;
  }

  float dw = (_centroids[index - 1].weight + _centroids[index].weight) / 2;

  // dw * q + weightSoFar
  weightSoFar += dw * (value - _centroids[index - 1].mean) / (_centroids[index].mean - _centroids[index - 1].mean);

  float totalWeight = 0.f;
  for (const auto &c :_centroids) {
    totalWeight += c.weight;
  }

  return weightSoFar / totalWeight;
}

void AgrrPDigest::merge_buffer_data() {
  // insert p-digest data
  _buffer.insert(_buffer.end(), _centroids.begin(), _centroids.begin() + _lastUsedCell);

  int32_t incomingCount = _buffer.size();

  // sort indexes based on comparing values in v
  std::sort(_buffer.begin(), _buffer.end(), [](const auto &lhs, const auto &rhs) {
    return lhs.mean < rhs.mean;
  });

  float totalWeight = 0.f;
  for (const auto &c :_buffer) {
    totalWeight += c.weight;
  }

  _lastUsedCell = 0;
  _centroids[0] = _buffer[0];

  float wSoFar = 0;
  float k1 = 0;
  // weight will contain all zeros
  float wLimit = totalWeight * PDigest::integratedQ(k1 + 1);

  for (int i = 1; i < incomingCount; ++i) {
    float proposedWeight = _centroids[_lastUsedCell].weight + _buffer[i].weight;

    float projectedW = wSoFar + proposedWeight;

    bool addThis = false;

    addThis = projectedW <= wLimit;

    if (addThis) {
      // next point will fit
      // so merge into existing centroid
      _centroids[_lastUsedCell].weight += _buffer[i].weight;
      _centroids[_lastUsedCell].mean = _centroids[_lastUsedCell].mean
          + (_buffer[i].mean - _centroids[_lastUsedCell].mean) * _buffer[i].weight / _centroids[_lastUsedCell].weight;

      _buffer[i].weight = 0;
    } else {
      // didn't fit ... move to next output, copy out first centroid
      wSoFar += _centroids[_lastUsedCell].weight;

      k1 = PDigest::integratedLocation(wSoFar / totalWeight);
      wLimit = totalWeight * PDigest::integratedQ(k1 + 1);

      _lastUsedCell++;

      _centroids[_lastUsedCell].mean = _buffer[i].mean;
      _centroids[_lastUsedCell].weight = _buffer[i].weight;

      _buffer[i].weight = 0;
    }
  }
  // points to next empty cell
  _lastUsedCell++;

  if (totalWeight > 0) {
    _min = std::min(_min, _centroids[0].mean);
    _max = std::max(_max, _centroids[_lastUsedCell - 1].mean);
  }

  _buffer.clear();
}

#endif // ENABLE_PDIGEST
