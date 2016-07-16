#include "stdafx.h"
#include "Query.h"

Query::Query(const std::string& url) : Query(string_util::split(url, "[/]+")) { }

Query::Query(const std::vector<std::string>& tokens) : Query(tokens[3], tokens[4]) {
   for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
      if ((*it) == "tile") {
         /*BUG fix spatial key*/
         const auto key = string_util::next_token(it);

         const uint32_t x = std::stoi(string_util::next_token(it));
         const uint32_t y = std::stoi(string_util::next_token(it));
         const uint8_t z = std::stoi(string_util::next_token(it));

         _resolution = std::stoi(string_util::next_token(it));
         _tile = tile_t(x, y, z);

      } else if ((*it) == "field") {
         _field.emplace(string_util::next_token(it));

      } else if ((*it) == "where") {
         std::vector<std::string> uri = string_util::split(string_util::next_token(it), std::regex("&"));

         for (auto clause : uri) {
            std::vector<std::string> literals = string_util::split(clause, std::regex("=|:"));

            if ((literals.size() - 1) <= 0) continue;

            for (size_t i = 1; i < literals.size(); i++) {
               _where[literals[0]].emplace_back(std::stoi(literals[i]));
            }

            std::sort(_where[literals[0]].begin(), _where[literals[0]].end());
         }
      } else if ((*it) == "tseries") {
         /*BUG fix temporal key*/
         auto key = string_util::next_token(it);

         temporal_t lower = std::stoul(string_util::next_token(it));
         temporal_t upper = std::stoul(string_util::next_token(it));

         _interval = interval_t(lower, upper);
      }
   }
}

Query::Query(const std::string& instance, const std::string& type) : _instance(instance) {
   if (type == "tile") {
      this->_type = TILE;
   } else if (type == "group") {
      this->_type = GROUP;
   } else if (type == "tseries") {
      this->_type = TSERIES;
   } else if (type == "scatter") {
      this->_type = SCATTER;
   } else if (type == "region") {
      this->_type = REGION;
   } else if (type == "mysql") {
      this->_type = MYSQL;
   }
}
