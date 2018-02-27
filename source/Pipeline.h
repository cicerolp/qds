//
// Created by cicerolp on 2/26/18.
//

#pragma once

#include "Query.h"

class Pipeline {
 public:
  Pipeline(const std::string &url);

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
  Query _source, _destination;
};
