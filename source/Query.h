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

   friend std::ostream& operator<<(std::ostream& os, const Query& query);

   struct query_t { };
   struct spatial_query_t : public query_t {
      // BUG fix multiple spatial dimenions
      uint8_t resolution {0};
      std::vector<region_t> region;
      std::vector<spatial_t> tile;
   };
   struct categorical_query_t : public query_t {
      bool field{ false };
      std::vector<categorical_t> where;
   };
   struct temporal_query_t : public query_t {
      interval_t interval;
   };

   inline bool eval(uint32_t key) const {
      return restrictions[key] != nullptr;
   }

   template<typename T>
   inline T* get(uint32_t key) const {
      return (T*)restrictions[key].get();
   }

   template<typename T>
   inline T* get(uint32_t key) {
      return (T*)restrictions[key].get();
   }

private:
   Query(const std::string& instance, const std::string& type);

   QueryType _type;
   std::string _instance;

   std::vector<std::unique_ptr<query_t>> restrictions;
};