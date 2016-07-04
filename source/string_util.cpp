#include "stdafx.h"
#include "string_util.h"

namespace string_util {

   std::vector<std::string> split(const std::string& input, const std::regex& regex) {
      static const std::sregex_token_iterator last;
      return std::vector<std::string>{ std::sregex_token_iterator{ input.begin(), input.end(), regex, -1 }, last };
   }

   std::vector<std::string> split(const std::string& input, const std::string& str_regex) {
      static const std::sregex_token_iterator last;
      std::regex regex(str_regex);
      return std::vector<std::string>{ std::sregex_token_iterator{ input.begin(), input.end(), regex, -1 }, last };
   }

} // namespace string_util