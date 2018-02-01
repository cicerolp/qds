//
// Created by cicerolp on 1/31/18.
//

#pragma once

class Payload {
 public:
  Payload(const DimensionSchema &schema) : _schema(schema) {}
  virtual ~Payload() = default;

  virtual std::vector<float> get_payload(Data &data, const Pivot &pivot) const = 0;

 protected:
  const DimensionSchema _schema;
};

class PayloadMerge {
 public:
  PayloadMerge() = default;
  virtual ~PayloadMerge() = default;

  virtual void merge(size_t payload_index, const Pivot &pivot) = 0;
  virtual void merge(size_t payload_index, pivot_it &it_lower, pivot_it &it_upper) = 0;
};
