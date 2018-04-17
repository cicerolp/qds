//
// Created by cicerolp on 4/17/18.
//

#include "stdafx.h"
#include "Clustering.h"

Clustering::Clustering(const std::string &url) {
  try {
    parse(url);
  } catch (...) {
    std::cerr << "error: invalid query" << std::endl;
  }
}

void Clustering::parse(const std::string &url) {
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
      }
    }
  }
}
