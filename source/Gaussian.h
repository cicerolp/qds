//
// Created by cicerolp on 2/2/18.
//

#pragma once

#include "Data.h"
#include "Payload.h"

#ifdef ENABLE_GAUSSIAN

class Gaussian : public Payload {
 public:
  Gaussian(const DimensionSchema &schema) : Payload(schema) {}

  std::vector<float> get_payload(Data &data, const Pivot &pivot) const override;
};

class AggrGaussian : public AgrrPayload {
 public:
  inline bool empty() const override {
    return count_i.size() == 0;
  }

  void merge(size_t payload_index, const Pivot &pivot, int32_t threshold) override;
  void merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper, int32_t threshold) override;

  float variance() const;
  float average() const;

 private:
  std::vector<float> count_i;
  std::vector<float> sum_i;
  std::vector<float> sum_square_i;
};

#endif // ENABLE_GAUSSIAN
