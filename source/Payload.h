//
// Created by cicerolp on 1/31/18.
//

#pragma once

class Payload {
 public:
  Payload(const DimensionSchema &schema) : _schema(schema) {}
  virtual ~Payload() = default;

  virtual std::vector<float> get_payload(Data &data, const Pivot &pivot) const = 0;

  void get_schema(rapidjson::Writer<rapidjson::StringBuffer> &writer) const {
    _schema.get_json(writer);
  };

 protected:
  const DimensionSchema _schema;
};

class AgrrPayload {
 public:
  AgrrPayload() = default;
  virtual ~AgrrPayload() = default;

  virtual inline bool empty() const = 0;

  virtual void merge(size_t payload_index, const Pivot &pivot, int32_t threshold) = 0;
  virtual void merge(size_t payload_index, const pivot_it &it_lower, const pivot_it &it_upper, int32_t threshold) = 0;
};
