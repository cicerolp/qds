//
// Created by cicerolp on 4/17/18.
//

#pragma once

class Clustering {
 public:
  Clustering(const std::string &url);

  void parse(const std::string &url);

  inline const std::string &get_dataset() const {
    return _dataset;
  }

 protected:
  std::string _dataset;
};

