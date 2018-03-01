//
// Created by cicerolp on 2/26/18.
//

#include "stdafx.h"
#include "Pipeline.h"

Pipeline::Pipeline(const std::string &url) {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(url, sep);

  for (auto &it : tokens) {
    std::vector<std::string> clausules;
    boost::split(clausules, it, boost::is_any_of("="));

    if (clausules.size() == 2) {

      auto &key = clausules[0];
      auto &value = clausules[1];

      if (key == "join") {
        _join = value;
        break;
      }
    }
  }

  const std::string source_template = "/source";
  const std::string dest_template = "/destination";

  auto source_idx = url.find(source_template);
  auto destination_idx = url.find(dest_template);

  if (std::string::npos != source_idx && std::string::npos != destination_idx) {
    std::string source, dest;

    if (source_idx < destination_idx) {
      source = url.substr(source_idx + source_template.size(), destination_idx - source_idx - source_template.size());
      dest = url.substr(destination_idx + dest_template.size());
    } else {
      source = url.substr(source_idx + source_template.size());
      dest = url.substr(destination_idx + dest_template.size(), source_idx - destination_idx - dest_template.size());
    }

    _source.parse(source);
    _destination.parse(dest);
  }
}
