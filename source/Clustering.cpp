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
      } else if (key == "cluster_by") {
        _cluster_by = value;
      } else if (key == "group_by") {
        auto equals_idx = value.find_first_of(".");

        if (std::string::npos != equals_idx) {
          _group_by = value.substr(0, equals_idx);
          _group_by_clausule = "/const=" + _group_by + "." + value.substr(equals_idx + 1) + "/group=" + _group_by;
        }

      } else if (key == "clusters") {
        _clusters = std::stoul(value);
      } else if (key == "iterations") {
        _iterations = std::stoul(value);
      } else if (key == "aggr") {
        auto equals_idx = value.find_first_of(".");

        if (std::string::npos != equals_idx) {
          aggr_expr aggr;

          aggr.first = value.substr(0, equals_idx);

          auto clausule = boost::trim_copy_if(value.substr(equals_idx + 1), boost::is_any_of("()"));

          boost::char_separator<char> sep(":");
          boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

          for (auto &v : tokens) {
            aggr.second.emplace_back(v);
          }

          _aggr.emplace_back(aggr);
        }
      }
    }
  }
}

std::string Clustering::get_aggr() const {
  std::stringstream aggr;

  for (auto &pair : _aggr) {
    for (auto &field: pair.second) {
      aggr << "/aggr=" << pair.first << "." << field << "_t";
    }
  }

  return aggr.str();
}
