#pragma once

#include "types.h"
#include "string_util.h"

class Query {
public:
   enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION };

   Query(const std::string& url);
   Query(const std::vector<std::string>& tokens);

   const QueryType& type() const {
      return _type;
   }

   const std::string& instance() const {
      return _instance;
   }

   bool eval_tile(const std::string& key) const {
      return !_tile.first.empty() && _tile.first == key;
   }

   const spatial_t& tile() const {
      return _tile.second;
   }

   bool eval_region(const std::string& key) const {
      return !_region.first.empty() && _region.first == key;
   }

   const region_t& region() const {
      return _region.second;
   }

   const uint8_t& resolution() const {
      return _resolution;
   }

   bool eval_interval(const std::string& key) const {
      return _interval.find(key) != _interval.end();
   }

   const interval_t& interval(const std::string& key) const {
      return _interval.at(key);
   }
   
   bool eval_field(const std::string& key) const {
      return _field.find(key) != _field.end();
   }

   bool eval_where(const std::string& key) const {
      return _where.find(key) != _where.end();
   }

   const std::vector<categorical_t>& where(const std::string& key) const {
      return _where.at(key);
   }

private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   uint8_t _resolution;
   std::pair<std::string, spatial_t> _tile;
   std::pair<std::string, region_t> _region;

   std::unordered_set<std::string> _field;
   std::unordered_map<std::string, interval_t> _interval;
   std::unordered_map<std::string, std::vector<categorical_t>> _where;   
};