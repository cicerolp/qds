//
// Created by cicerolp on 3/9/18.
//

#pragma once

#include "Pipeline.h"

class AugmentedSeries {
 public:
  AugmentedSeries(const std::string &url);

  Pipeline& get_pipeline() const {
    return *_pipeline;
  }

  const std::string& get_dimension() const {
    return _dimension;
  }

  const std::vector<interval_t>& get_bounds() const {
    return _bounds;
  }

 protected:
  void parse_series(const std::string &str);

  std::string _dimension;
  std::vector<interval_t> _bounds;
  std::unique_ptr<Pipeline> _pipeline;
};