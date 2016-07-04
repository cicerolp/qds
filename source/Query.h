#pragma once

#include "string_util.h"

class Query {
public:
   enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION };

   Query(const std::string& url);
   Query(const std::vector<std::string>& tokens);
   ~Query() = default;

   inline bool evalGroup(const std::string& key) const {
      return _filed.find(key) != _filed.end();
   }

   inline bool evalWhere(const std::string& key) const {
      return _where.find(key) != _where.end();
   }

private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   std::unordered_set<std::string> _filed;
   std::unordered_map<std::string, std::vector<int>> _where;

   inline std::string nextToken(std::vector<std::string>::const_iterator& it) { return  *(++it); }
};