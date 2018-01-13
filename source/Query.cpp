#include "Query.h"

Query::Query(const std::string &url) : _url(url) {
  try {
    parse(url);
  } catch (...) {
    std::cerr << "error: invalid query" << std::endl;
  }
}

void Query::parse(const std::string &url) {
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


        const auto equals_idx = value.find_first_of(".");

        if (std::string::npos != equals_idx) {
          _aggr.first = value.substr(0, equals_idx);
          _aggr.second = value.substr(equals_idx + 1);
        } else {
          _aggr.first = value;
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
std::ostream &operator<<(std::ostream &os, const Query &query) {
  os << query._url;
  return os;
}

