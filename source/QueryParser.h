//
// Created by cicerolp on 1/11/18.
//

#pragma once

class QueryParser {
 public:
  using const_t = std::pair<std::string, std::string>;

  QueryParser(const std::string &url);

  inline const_t get_const(const std::string &dim) {
    auto pair = _constraints.find(dim);

    if (pair == _constraints.end()) {
      return std::make_pair("", "");
    } else {
      return pair->second;
    }
  };

 private:
  void parse(const std::string &url);

  std::string _url;

  std::string _dataset;

  // [dimesion name] -> constraints
  std::unordered_map<std::string, const_t> _constraints;

  // [dimesion name] -> group_by
  std::string _group_by;

  // [dimesion name] -> aggr
  std::pair<std::string, std::string> _aggr;
};
