#pragma once

#include "types.h"
#include "string_util.h"

class Query {
public:
   enum QueryType { TILE, GROUP, TSERIES, SCATTER, MYSQL, REGION };

   Query(const std::string& url);
   Query(const std::vector<std::string>& tokens);

   inline const QueryType& type() const {
      return _type;
   }

   inline const std::string& instance() const {
      return _instance;
   }

   inline bool eval_tile(uint32_t key) const {
      return _tile[key].first;
   }

   inline const spatial_t& tile(uint32_t key) const {
      return _tile[key].second;
   }

   inline bool eval_region(uint32_t key) const {
      return _region[key].first;
   }

   inline const region_t& region(uint32_t key) const {
      return _region[key].second;
   }

   inline const uint8_t& resolution() const {
      return _resolution;
   }

   inline bool eval_interval(uint32_t key) const {
      return _interval[key].first;
   }

   inline const interval_t& interval(uint32_t key) const {
      return _interval[key].second;
   }
   
   inline bool eval_field(uint32_t key) const {
      return _field[key];
   }

   inline bool eval_where(uint32_t key) const {
      return _where[key].first;
   }

   inline const std::vector<categorical_t>& where(uint32_t key) const {
      return _where[key].second;
   }

   friend std::ostream& operator<<(std::ostream& os, const Query& query);

private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   uint8_t _resolution;
   
   std::vector<std::pair<bool, spatial_t>> _tile;
   std::vector<std::pair<bool, region_t>> _region;

   std::vector<bool> _field;
   std::vector<std::pair<bool, std::vector<categorical_t>>> _where;

   std::vector<std::pair<bool, interval_t>> _interval;
};