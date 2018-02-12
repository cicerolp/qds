//
// Created by cicerolp on 2/2/18.
//

#include "stdafx.h"
#include "Gaussian.h"
#include "Pivot.h"
#include "NDS.h"

#ifdef ENABLE_GAUSSIAN

std::vector<float> Gaussian::get_payload(Data &data, const Pivot &pivot) const {
  // sum, sum_square
  std::vector<float> payload(2, 0.f);

  for (auto p = pivot.front(); p < pivot.back(); ++p) {
    auto elt = data.payload(_schema.offset, p);

    // sum
    payload[0] += elt;

    // sum_square
    payload[1] += elt * elt;
  }

  return payload;
}

void AggrGaussian::merge(size_t payload_index, const Pivot &pivot) {
  const auto &payload = pivot.get_payload(payload_index);

  count_i.emplace_back(pivot.size());
  sum_i.emplace_back(payload[0]);
  sum_square_i.emplace_back(payload[1]);
}

void AggrGaussian::merge(size_t payload_index, pivot_it &it_lower, pivot_it &it_upper) {

  // insert payload data
  while (it_lower != it_upper) {
    auto &payload = (*it_lower).get_payload(payload_index);

    count_i.emplace_back((*it_lower).size());
    sum_i.emplace_back(payload[0]);
    sum_square_i.emplace_back(payload[1]);

    ++it_lower;
  }
}

float AggrGaussian::variance() const {
  // sum(xi ^ 2) / n - average(x) ^ 2

  float sum = 0.f;
  float count = 0.f;
  float sum_square = 0.f;

  for (auto i = 0; i < count_i.size(); ++i) {
    sum += sum_i[i];
    count += count_i[i];
    sum_square += sum_square_i[i];
  }

  float average = sum / count;

  return sum_square / count - (average * average);
}

float AggrGaussian::average() const {
  float count = std::accumulate(count_i.begin(), count_i.end(), 0);
  float sum = std::accumulate(sum_i.begin(), sum_i.end(), 0);

  return sum / count;
}

#endif // ENABLE_GAUSSIAN
