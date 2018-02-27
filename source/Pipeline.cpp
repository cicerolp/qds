//
// Created by cicerolp on 2/26/18.
//

#include "stdafx.h"
#include "Pipeline.h"

Pipeline::Pipeline(const std::string &url) {
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
