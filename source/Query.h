#pragma once

#include "types.h"
#include "string_util.h"

class Query {
public:
   const std::string& instance() const {
      return _instance;
   }

   const std::pair<std::string, spatial_t>& tile() const {
      return _tile;
   }
   const uint8_t& resolution() const {
      return _resolution;
   }

   const std::unordered_set<std::string>& field() const {
      return _field;
   }

   const std::unordered_map<std::string, interval_t>& interval() const {
      return _interval;
   }
   
   const std::unordered_map<std::string, std::vector<categorical_t>>& where() const {
      return _where;
   }

   enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION };

   Query(const std::string& url);
   Query(const std::vector<std::string>& tokens);

   friend std::ostream& operator<< (std::ostream& stream, const Query& query);   

private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   uint8_t _resolution;
   std::pair<std::string, spatial_t> _tile;

   std::unordered_set<std::string> _field;
   std::unordered_map<std::string, interval_t> _interval;
   std::unordered_map<std::string, std::vector<categorical_t>> _where;   
};