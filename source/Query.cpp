#include "stdafx.h"
#include "Query.h"

Query::Query(const std::string& url) : Query(string_util::split(url, "[/]+")) { }

Query::Query(const std::vector<std::string>& tokens) : Query(tokens[3], tokens[4]) {
   for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
      if ((*it) == "field") {
         _field.emplace(nextToken(it));

      } else if ((*it) == "where") {
         std::vector<std::string> uri = string_util::split(nextToken(it), std::regex("&"));

         for (auto clause : uri) {
            std::vector<std::string> literals = string_util::split(clause, std::regex("=|:"));

            if ((literals.size() - 1) <= 0) continue;

            for (size_t i = 1; i < literals.size(); i++) {
               _where[literals[0]].emplace_back(std::stoi(literals[i]));               
            }

            std::sort(_where[literals[0]].begin(), _where[literals[0]].end());
         }
      }
   }
}

Query::Query(const std::string& instance, const std::string& type) : _instance(instance) {
   if (type == "tile") {
      this->_type = TILE;
   }
   else if (type == "group") {
      this->_type = GROUP;
   }
   else if (type == "tseries") {
      this->_type = TSERIES;
   }
   else if (type == "scatter") {
      this->_type = SCATTER;
   }
   else if (type == "region") {
      this->_type = REGION;
   }
   else if (type == "mysql") {
      this->_type = MYSQL;
   }
}
