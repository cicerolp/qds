#include "stdafx.h"
#include "Query.h"

Query::Query(const std::string& url) : Query(string_util::split(url, "[/]+")) { }

Query::Query(const std::vector<std::string>& tokens) : Query(tokens[3], tokens[4]) {
   for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
      if ((*it) == "tile") {
         auto key = std::stoul(string_util::next_token(it));

         if (!restrictions[key]) restrictions[key] = std::make_unique<spatial_query_t>();

         auto x = std::stoi(string_util::next_token(it));
         auto y = std::stoi(string_util::next_token(it));
         auto z = std::stoi(string_util::next_token(it));

         get<spatial_query_t>(key)->tile.emplace_back(x, y, z);
         get<spatial_query_t>(key)->resolution = std::stoi(string_util::next_token(it));

      } else if ((*it) == "region") {
         auto key = std::stoul(string_util::next_token(it));

         if (!restrictions[key]) restrictions[key] = std::make_unique<spatial_query_t>();

         auto z = std::stoi(string_util::next_token(it));
         auto x0 = std::stoi(string_util::next_token(it));
         auto y0 = std::stoi(string_util::next_token(it));
         auto x1 = std::stoi(string_util::next_token(it));
         auto y1 = std::stoi(string_util::next_token(it));

         get<spatial_query_t>(key)->region.emplace_back(x0, y0, x1, y1, z);

      } else if ((*it) == "field") {
         auto key = std::stoul(string_util::next_token(it));

         if (!restrictions[key]) restrictions[key] = std::make_unique<categorical_query_t>();

         get<categorical_query_t>(key)->field = true;

      } else if ((*it) == "where") {
         std::vector<std::string> uri = string_util::split(string_util::next_token(it), std::regex("&"));

         for (auto clause : uri) {
            std::vector<std::string> literals = string_util::split(clause, std::regex("=|:"));
            auto key = std::stoul(literals[0]);

            if (!restrictions[key]) restrictions[key] = std::make_unique<categorical_query_t>();

            if ((literals.size() - 1) <= 0) continue;

            for (size_t i = 1; i < literals.size(); i++) {
               get<categorical_query_t>(key)->where.emplace_back(std::stoi(literals[i]));
            }

            std::sort(get<categorical_query_t>(key)->where.begin(), get<categorical_query_t>(key)->where.end());
         }
      } else if ((*it) == "tseries") {
         auto key = std::stoul(string_util::next_token(it));

         if (!restrictions[key]) restrictions[key] = std::make_unique<temporal_query_t>();

         temporal_t lower = std::stoul(string_util::next_token(it));
         temporal_t upper = std::stoul(string_util::next_token(it));

         get<temporal_query_t>(key)->interval = interval_t(lower, upper);
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

   // BUG fix
   restrictions.resize(8);
}

std::ostream& operator<<(std::ostream& os, const Query& query) {
   os << "/" + query.instance();

   switch (query.type()) {
      case Query::TILE: os << "/tile";
         break;
      case Query::GROUP: os << "/group";
         break;
      case Query::TSERIES: os << "/tseries";
         break;
      case Query::SCATTER: os << "/scatter";
         break;
      case Query::REGION: os << "/region";
         break;
      case Query::MYSQL: os << "/mysql";
         break;
   }

   // BUG fix
   /*// /tile/key/x/y/z/r
   for (int i = 0; i < query._tile.size(); ++i) {
      if (query.eval_tile(i)) {
         os << "/tile/" << i << "/" << query.tile(i) << "/" << (uint32_t)query.resolution();
      }
   }

   // /region/key/z/x0/y0/x1/y1
   for (int i = 0; i < query._region.size(); ++i) {
      if (query.eval_region(i)) {
         os << "/region/" << i << "/" << query.region(i);
      }
   }

   // /field/<category>
   for (int i = 0; i < query._field.size(); ++i) {
      if (query.eval_field(i)) {
         os << "/field/" << i;
      }
   }

   // /where/<category>=<[value]:[value]...:[value]>&<category>=<[value]:[value]...:[value]>
   std::string where_stream;
   for (int i = 0; i < query._where.size(); ++i) {
      if (query.eval_where(i)) {

         where_stream += std::to_string(i) + "=";

         for (auto& value : query.where(i)) {
            where_stream += std::to_string(value) + ":";
         }

         where_stream = where_stream.substr(0, where_stream.size() - 1);
         where_stream += "&";
      }
   }

   if (!where_stream.empty()) {
      where_stream = "/where/" + where_stream;
      where_stream = where_stream.substr(0, where_stream.size() - 1);
      os << where_stream;
   }

   // /tseries/key/
   for (int i = 0; i < query._interval.size(); ++i) {
      if (query.eval_interval(i)) {
         os << "/tseries/" << i << "/" << query.interval(i);
      }
   }*/

   return os;
}
