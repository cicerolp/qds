//
// Created by cicerolp on 6/16/18.
//

#include "Query.h"

#pragma once

namespace util {

inline Query get_from_url(const std::string &line) {
  std::string query_str = "/rest/query";

  try {
    if (line.empty()) throw std::invalid_argument("empty line");

    auto record = string_util::split(line, ",");

    if (record.size() != 3) throw std::invalid_argument("malformed url");

    auto url = string_util::split(record[0], "/");

    if (url[5] == "tile") {
      query_str += "/tile";

      // zoom/x0/y0/x1/y1
      query_str += "/" + url[6] + "/" + url[8] + "/" + url[9] + "/" + url[8] + "/" + url[9];

      // resolution
      query_str += "/resolution/" + url[7];

    } else if (url[5] == "query") {
      query_str += "/region";

      // zoom
      if (url[7] == "undefined") {
        query_str += "/1";
      } else {
        query_str += "/" + url[7];
      }

      // x0/y0/x1/y1
      query_str += "/" + url[8] + "/" + url[9] + "/" + url[10] + "/" + url[11];
    }
  } catch (std::invalid_argument) {
    std::cerr << "error: invalid nanocubes query [" << line << "]" << std::endl;
  }

  return Query(query_str);
}

inline std::vector<Query> get_from_file(const std::string &file) {
  std::vector<Query> queries;

  std::ifstream infile(file);

  while (!infile.eof()) {

    std::string line;
    std::getline(infile, line);

    if (line.empty()) continue;

    queries.emplace_back(get_from_url(line));
  }

  infile.close();

  return queries;
}

} // namespace util