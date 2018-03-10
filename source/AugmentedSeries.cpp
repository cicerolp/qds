//
// Created by cicerolp on 3/9/18.
//

#include "stdafx.h"
#include "AugmentedSeries.h"

// series=[dimension_name].([lower_bound]:[interval_width]:[num_intervals]:[stride])
AugmentedSeries::AugmentedSeries(const std::string &url) {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(url, sep);

  for (auto &it : tokens) {
    std::vector<std::string> clausules;
    boost::split(clausules, it, boost::is_any_of("="));

    if (clausules.size() == 2) {

      auto &key = clausules[0];
      auto &value = clausules[1];

      if (key == "series") {
        auto equals_idx = value.find_first_of(".");

        if (std::string::npos != equals_idx) {
          std::string type = value.substr(0, equals_idx);
          std::string expr = value.substr(equals_idx + 1);

          _dimension = type;
          parse_series(expr);
        }
        break;
      }
    }
  }

  _pipeline = std::make_unique<Pipeline>(url);
}

void AugmentedSeries::parse_series(const std::string &str) {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  std::vector<std::string> tokens;
  boost::split(tokens, clausule, boost::is_any_of(":"));

  temporal_t lower = std::stoi(tokens[0]);
  temporal_t width = std::stoi(tokens[1]);
  uint32_t num_intervals = std::stoi(tokens[2]);
  uint32_t stride = std::stoi(tokens[3]);

  for (int i = 0; i < num_intervals; ++i) {
    _bounds.emplace_back(lower, lower + width);
    lower += stride;
  }
}
