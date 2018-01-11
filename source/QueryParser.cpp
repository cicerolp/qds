//
// Created by cicerolp on 1/11/18.
//

#include "stdafx.h"
#include "QueryParser.h"

QueryParser::QueryParser(const std::string &url) {
  std::cout << url << std::endl;
  parse(url);
}

void QueryParser::parse(const std::string &url) {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(url, sep);

  for (auto &it : tokens) {
    std::vector<std::string> clausules;
    boost::split(clausules, it, boost::is_any_of("="));

    if (clausules.size() == 2) {

      auto &key = clausules[0];
      auto &value = clausules[1];

      if (key == "dataset") {
        _dataset = value;

      } else if (key == "aggr") {

        std::vector<std::string> aggr_str;
        boost::split(aggr_str, value, boost::is_any_of("."));

        _aggr.first = aggr_str[0];

        if (aggr_str.size() == 2) {
          _aggr.second = aggr_str[1];
        }

      } else if (key == "group") {
        _group_by = value;

      } else if (key == "const") {

        std::vector<std::string> const_str;
        boost::split(const_str, value, boost::is_any_of("."));

        _constraints[const_str[0]] = std::make_pair(const_str[1], const_str[2]);

      }
    }
  }
}
