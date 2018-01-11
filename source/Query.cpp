#include "Query.h"

void Query::initialize(const std::string &instance, const std::string &output, const std::string &aggregation) {
  // dataset instance
  _instance = instance;

  // output_type
  _output_str = output;
  if (output == "count") {
    _output = COUNT;
  } else if (output == "quantile") {
    _output = QUANTILE;
  }

  // query_aggregation
  _aggregation_str = aggregation;
  if (aggregation == "tile") {
    _aggregation = TILE;
  } else if (aggregation == "group") {
    _aggregation = GROUP;
  } else if (aggregation == "tseries") {
    _aggregation = TSERIES;
  } else if (aggregation == "scatter") {
    _aggregation = SCATTER;
  } else if (aggregation == "none") {
    _aggregation = NONE;
  }
}

Query::Query(const std::string &url) {
  std::cout << url << std::endl;
  parse(url);
}

Query::Query(const std::vector<std::string> &tokens) {
  // initialize query parameters
  initialize(tokens[3], tokens[4], tokens[5]);

  for (auto it = tokens.begin() + 6; it != tokens.end(); ++it) {
    if ((*it) == "quantile") {
      _quantiles.emplace_back(std::stof(string_util::next_token(it)));

    } else if ((*it) == "region") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<spatial_restriction_t>(key);

      auto z = std::stoi(string_util::next_token(it));
      auto x0 = std::stoi(string_util::next_token(it));
      auto y0 = std::stoi(string_util::next_token(it));
      auto x1 = std::stoi(string_util::next_token(it));
      auto y1 = std::stoi(string_util::next_token(it));

      r->region = std::make_unique<region_t>(x0, y0, x1, y1, z);

    } else if ((*it) == "tile") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<spatial_restriction_t>(key);

      auto x = std::stoi(string_util::next_token(it));
      auto y = std::stoi(string_util::next_token(it));
      auto z = std::stoi(string_util::next_token(it));

      r->tile = std::make_unique<spatial_t>(x, y, z);

      // TODO fix resolution
      //r->resolution = std::stoi(string_util::next_token(it));

    } else if ((*it) == "field") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<categorical_restriction_t>(key);

      r->field = true;

    } else if ((*it) == "where") {
      std::vector<std::string> uri =
          string_util::split(string_util::next_token(it), std::regex("&"));

      for (auto clause : uri) {
        std::vector<std::string> literals =
            string_util::split(clause, std::regex("=|:"));
        auto key = std::stoul(literals[0]);

        if ((literals.size() - 1) <= 0) continue;

        std::vector<categorical_t> where;
        for (size_t i = 1; i < literals.size(); i++) {
          where.emplace_back(std::stoi(literals[i]));
        }

        std::sort(where.begin(), where.end());

        if (where.size() != 0) {
          auto r = get<categorical_restriction_t>(key);
          r->where.assign(where.begin(), where.end());
        }
      }

    } else if ((*it) == "tseries") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<temporal_restriction_t>(key);

      temporal_t lower = std::stoul(string_util::next_token(it));
      temporal_t upper = std::stoul(string_util::next_token(it));

      r->interval = interval_t(lower, upper);
    }
  }
}

void Query::print(std::ostream &os) const {
  os << "/" + _instance;

  os << "/" + _output_str;

  os << "/" + _aggregation_str;

  for (auto &pair : _restrictions) {
    uint32_t id = pair.first;
    pair.second->print(id, os);
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

