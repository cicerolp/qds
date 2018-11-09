#include "stdafx.h"

#pragma once

namespace util {

std::vector<std::string> split(const std::string& input,
                               const std::regex& regex);
std::vector<std::string> split(const std::string& input,
                               const std::string& str_regex);

std::string next_token(std::vector<std::string>::const_iterator& it);

}  // namespace util