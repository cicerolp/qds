#pragma once

#include "string_util.h"

class Query {
public:
   enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION };

   Query(const std::string& url);
   Query(const std::vector<std::string>& tokens);
   
private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   std::unordered_set<std::string> _field;
   std::unordered_map<std::string, std::vector<categorical_t>> _where;

   inline std::string nextToken(std::vector<std::string>::const_iterator& it) { return  *(++it); }
};