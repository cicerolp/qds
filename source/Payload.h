//
// Created by cicerolp on 1/31/18.
//

#pragma once

class Payload {
 public:
  Payload(const DimensionSchema &schema) : _schema(schema) { }

  virtual ~Payload() = default;

 protected:
  const DimensionSchema _schema;
};
