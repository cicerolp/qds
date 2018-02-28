//
// Created by cicerolp on 2/26/18.
//

#pragma once

#include "Query.h"

class Pipeline {
 public:
  Pipeline(const std::string &url);

  inline int32_t get_threshold() const {
    return _threshold;
  }

  inline const std::string &get_join() const {
    return _join;
  }

  inline const std::string &get_dataset() const {
    return _source.get_dataset();
  }

  inline const Query &get_source() const {
    return _source;
  }

  inline const Query &get_dest() const {
    return _destination;
  }

 protected:
  int32_t _threshold{-1};
  std::string _join{"inner_join"};
  Query _source, _destination;
};
