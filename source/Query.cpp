#include "Query.h"

Query::Query(const std::string &url) {
  try {
    parse(url);
  } catch (...) {
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
std::ostream &operator<<(std::ostream &os, const Query &query) {
  // TODO output stream
  /*os << "_url: " << query._url << " _dataset: " << query._dataset << " _constraints: " << query._constraints
     << " _group_by: " << query._group_by << " _aggr: " << query._aggr;*/
  return os;
}

