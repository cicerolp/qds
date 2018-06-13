//
// Created by cicerolp on 6/13/18.
//

#pragma once

#include "Data.h"
#include "Payload.h"

#ifdef ENABLE_RAW

class Raw : public Payload {
 public:
  Raw(const DimensionSchema &schema) : Payload(schema) {}

  std::vector<float> get_payload(Data &data, const Pivot &pivot) override;
};

class AggrRaw : public AgrrPayload {
 public:
  virtual ~AggrRaw() = default;

  inline bool empty() const override {
    return _payload.size() == 0;
  }

  uint32_t merge(size_t payload_index, const Pivot &pivot) override;
  uint32_t merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper) override;

  float quantile(float q);
  float inverse(float value);

 private:
  void sort_data();

  bool _sorted{false};
  std::vector<float> _payload;
};

#endif // ENABLE_RAW