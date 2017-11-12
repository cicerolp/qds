#include "Query.h"

Query::Query(const std::string &url) : Query(string_util::split(url, "[/]+")) {}

Query::Query(const std::vector<std::string> &tokens) : Query(tokens[3], tokens[4]) {

  for (auto it = tokens.begin() + 5; it != tokens.end(); ++it) {
    if ((*it) == "tile") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<spatial_query_t>(key);

      auto x = std::stoi(string_util::next_token(it));
      auto y = std::stoi(string_util::next_token(it));
      auto z = std::stoi(string_util::next_token(it));

      r->tile.emplace_back(x, y, z);
      r->resolution = std::stoi(string_util::next_token(it));

    } else if ((*it) == "region") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<spatial_query_t>(key);

      auto z = std::stoi(string_util::next_token(it));
      auto x0 = std::stoi(string_util::next_token(it));
      auto y0 = std::stoi(string_util::next_token(it));
      auto x1 = std::stoi(string_util::next_token(it));
      auto y1 = std::stoi(string_util::next_token(it));

      r->region.emplace_back(x0, y0, x1, y1, z);

    } else if ((*it) == "field") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<categorical_query_t>(key);

      r->field = true;

    } else if ((*it) == "where") {
      std::vector<std::string> uri =
          string_util::split(string_util::next_token(it), std::regex("&"));

      for (auto clause : uri) {
        std::vector<std::string> literals =
            string_util::split(clause, std::regex("=|:"));
        auto key = std::stoul(literals[0]);

        if ((literals.size() - 1) <= 0) continue;

        std::vector<categorical_t> where;
        for (size_t i = 1; i < literals.size(); i++) {
          where.emplace_back(std::stoi(literals[i]));
        }

        std::sort(where.begin(), where.end());

        if (where.size() != 0) {
          auto r = get<categorical_query_t>(key);
          r->where.assign(where.begin(), where.end());
        }
      }
    } else if ((*it) == "tseries") {
      auto key = std::stoul(string_util::next_token(it));

      auto r = get<temporal_query_t>(key);

      temporal_t lower = std::stoul(string_util::next_token(it));
      temporal_t upper = std::stoul(string_util::next_token(it));

      r->interval = interval_t(lower, upper);
    }
  }
}

Query::Query(const std::string &instance, const std::string &type)
    : _instance(instance) {
  if (type == "tile") {
    _type = TILE;
  } else if (type == "group") {
    _type = GROUP;
  } else if (type == "tseries") {
    _type = TSERIES;
  } else if (type == "scatter") {
    _type = SCATTER;
  } else if (type == "region") {
    _type = REGION;
  } else if (type == "mysql") {
    _type = MYSQL;
  } else if (type == "quantile") {
    _type = QUANTILE;
  }
}

std::ostream &operator<<(std::ostream &os, const Query &query) {
  os << "/" + query.instance();

  switch (query.type()) {
    case Query::TILE:os << "/tile";
      break;
    case Query::GROUP:os << "/group";
      break;
    case Query::TSERIES:os << "/tseries";
      break;
    case Query::SCATTER:os << "/scatter";
      break;
    case Query::REGION:os << "/region";
      break;
    case Query::MYSQL:os << "/mysql";
      break;
  }

  for (auto &pair : query._restrictions) {
    auto &r = pair.second;

    uint32_t index = pair.first;

    switch (r->id) {
      case Query::query_t::spatial: {
        auto spatial = static_cast<Query::spatial_query_t &>(*r.get());
        // /region/key/z/x0/y0/x1/y1
        for (auto &region : spatial.region) {
          os << "/region/" << index << "/" << region;
        }
        // /tile/key/x/y/z/r
        for (auto &tile : spatial.tile) {
          os << "/tile/" << index << "/" << tile << "/" << spatial.resolution;
        }
      }
        break;
      case Query::query_t::categorical: {
        auto categorical = static_cast<Query::categorical_query_t &>(*r.get());
        // /field/<key>
        if (categorical.field) {
          os << "/field/" << index;
        }

        // /where/<category>=<[value]:[value]...:[value]>&<category>=<[value]:[value]...:[value]>
        if (categorical.where.size()) os << "/where/" << index << "=";
        std::string where_stream;
        for (auto &value : categorical.where) {
          where_stream += std::to_string(value) + ":";
        }
        where_stream = where_stream.substr(0, where_stream.size() - 1);
        if (categorical.where.size()) os << where_stream;
      }
        break;

      case Query::query_t::temporal: {
        auto temporal = static_cast<Query::temporal_query_t &>(*r.get());
        // /tseries/key/from_date/to_date
        os << "/tseries/" << index << "/" << temporal.interval;
      }
        break;
      default:break;
    }
  }

  return os;
}
