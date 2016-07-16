#include "stdafx.h"
#include "Query.h"

Query::Query(const std::string& url) : Query(string_util::split(url, "[/]+")) { }

Query::Query(const std::vector<std::string>& tokens) : Query(tokens[3], tokens[4]) {
   for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
      if ((*it) == "tile") {
         const auto key = string_util::next_token(it);

         const uint32_t x = std::stoi(string_util::next_token(it));
         const uint32_t y = std::stoi(string_util::next_token(it));
         const uint8_t z = std::stoi(string_util::next_token(it));

         _resolution = std::stoi(string_util::next_token(it));
         _tile = { key, tile_t(x, y, z) };

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
         auto key = string_util::next_token(it);

         temporal_t lower = std::stoul(string_util::next_token(it));
         temporal_t upper = std::stoul(string_util::next_token(it));

         _interval.emplace(key, interval_t(lower, upper));
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

std::ostream & operator<<(std::ostream & stream, const Query & query) {

   stream << "/" + query._instance;

   switch (query._type) {
      case Query::TILE: stream << "/tile"; break;
      case Query::GROUP: stream << "/group"; break;
      case Query::TSERIES: stream << "/tseries"; break;
      case Query::SCATTER: stream << "/scatter"; break;
      case Query::REGION: stream << "/region"; break;
      case Query::MYSQL: stream << "/mysql"; break;
   }

   // /tile/key/x/y/z/r
   {
      std::string r = std::to_string(query.resolution());
      stream << "/tile/" + query.tile().first + "/" << query.tile().second << "/" + r;
   }

   // /region/key/z/x0/y0/x1/y1
   /*for (size_t i = 0; i < query._region.size(); ++i) {
      if (query._region[i].isValid()) {
         std::string key = std::to_string(i);
         std::string z = std::to_string(query.zoom);

         stream << "/region/" + key + "/" + z + "/" << query._region[i];
      }
   }*/


   // /field/<category>
   for (auto& field : query.field()) {
      stream << "/field/" + field;
   }

   // /where/<category>=<[value]:[value]...:[value]>&<category>=<[value]:[value]...:[value]>
   if (query.where().size() != 0) {
      std::string where_stream = "/where/";

      for (auto& where : query._where) {
         where_stream += where.first + "=";

         for (size_t i = 0; i < where.second.size(); ++i) {
            if (where.second[i]) where_stream += std::to_string(i) + ":";
         }
         where_stream = where_stream.substr(0, where_stream.size() - 1);
         where_stream += "&";
      }

      where_stream = where_stream.substr(0, where_stream.size() - 1);
      stream << where_stream;
   }

   // /tseries/key/
   for (auto& tseries : query.interval()) {
      stream << "/tseries/" + tseries.first + "/" + std::to_string(tseries.second.bound[0]) + "/" + std::to_string(tseries.second.bound[1]);
   }

   return stream;
}