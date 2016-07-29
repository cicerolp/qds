#include "stdafx.h"
#include "Query.h"

Query::Query(const std::string& url) : Query(string_util::split(url, "[/]+")) { }

Query::Query(const std::vector<std::string>& tokens) : Query(tokens[3], tokens[4]) {
   for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
      if ((*it) == "tile") {
         auto key = std::stoul(string_util::next_token(it));

         uint32_t x = std::stoi(string_util::next_token(it));
         uint32_t y = std::stoi(string_util::next_token(it));
         uint8_t z = std::stoi(string_util::next_token(it));

         _resolution = std::stoi(string_util::next_token(it));

         _tile[key].first = true;
         _tile[key].second = spatial_t(x, y, z);

      } else if ((*it) == "region") {
         auto key = std::stoul(string_util::next_token(it));

         uint8_t z = std::stoi(string_util::next_token(it));

         uint32_t x0 = std::stoi(string_util::next_token(it));
         uint32_t y0 = std::stoi(string_util::next_token(it));
         uint32_t x1 = std::stoi(string_util::next_token(it));
         uint32_t y1 = std::stoi(string_util::next_token(it));

         _region[key].first = true;
         _region[key].second = region_t(x0, y0, x1, y1, z);

      } else if ((*it) == "field") {
         auto key = std::stoul(string_util::next_token(it));

         _field[key] = true;

      } else if ((*it) == "where") {
         std::vector<std::string> uri = string_util::split(string_util::next_token(it), std::regex("&"));

         for (auto clause : uri) {
            std::vector<std::string> literals = string_util::split(clause, std::regex("=|:"));
            auto key = std::stoul(literals[0]);

            if ((literals.size() - 1) <= 0) continue;

            for (size_t i = 1; i < literals.size(); i++) {
               _where[key].second.emplace_back(std::stoi(literals[i]));
            }

            _where[key].first = true;
            std::sort(_where[key].second.begin(), _where[key].second.end());
         }
      } else if ((*it) == "tseries") {
         auto key = std::stoul(string_util::next_token(it));

         temporal_t lower = std::stoul(string_util::next_token(it));
         temporal_t upper = std::stoul(string_util::next_token(it));

         _interval[key].first = true;
         _interval[key].second = interval_t(lower, upper);
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
   _tile.resize(1, std::make_pair(false, spatial_t()));
   _region.resize(1, std::make_pair(false, region_t()));

   _field.resize(2, false);
   _where.resize(2, std::make_pair(false, std::vector<categorical_t>()));

   _interval.resize(1, std::make_pair(false, interval_t()));
}

std::ostream& operator<<(std::ostream& os, const Query& query) {
   /*os << "/" + query.instance();

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

   // /tile/key/x/y/z/r
   for (auto& pair : query._tile) {
      os << "/tile/" + pair.first + "/" << pair.second << "/" << (uint32_t)query.resolution();
   }

   // /region/key/z/x0/y0/x1/y1
   for (auto& pair : query._region) {
      os << "/region/" << pair.first << "/" << pair.second;
   }
   
   // /field/<category>
   for (auto& field : query._field) {
      os << "/field/" << field;
   }

   // /where/<category>=<[value]:[value]...:[value]>&<category>=<[value]:[value]...:[value]>
   if (query._where.size() != 0) {
      std::string where_stream = "/where/";

      for (auto& pair : query._where) {
         where_stream += pair.first + "=";

         for (auto& value : pair.second) {
            where_stream += std::to_string(value) + ":";
         }
         where_stream = where_stream.substr(0, where_stream.size() - 1);
         where_stream += "&";
      }

      where_stream = where_stream.substr(0, where_stream.size() - 1);
      os << where_stream;
   }

   // /tseries/key/
   for (auto& pair : query._interval) {
      os << "/tseries/" << pair.first << "/" << pair.second;
   }*/

   return os;
}
