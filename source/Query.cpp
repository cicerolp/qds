#include "Query.h"
#include "Spatial.h"
#include "Temporal.h"
#include "Categorical.h"

Query::Query(const std::string &url) : _url(url) {
  try {
    parse(url);
  } catch (...) {
    std::cerr << "error: invalid query" << std::endl;
  }
}

void Query::parse(const std::string &url) {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(url, sep);

  for (auto &it : tokens) {
    std::vector<std::string> clausules;
    boost::split(clausules, it, boost::is_any_of("="));

    if (clausules.size() == 2) {

      auto &key = clausules[0];
      auto &value = clausules[1];

      if (key == "dataset") {
        _dataset = value;

      } else if (key == "aggr") {
        // [type, [dimension, values]]

        auto equals_idx = value.find_first_of(".");

        if (std::string::npos != equals_idx) {

          std::string type = value.substr(0, equals_idx);

          std::string expr = value.substr(equals_idx + 1);
          equals_idx = expr.find_first_of(".");

          if (std::string::npos != equals_idx) {
            _aggr.emplace_back(type, std::make_pair(expr.substr(0, equals_idx), expr.substr(equals_idx + 1)));
          } else {
            _aggr.emplace_back(type, std::make_pair(expr, ""));
          }
        } else {
          _aggr.emplace_back(value, clausule());
        }

      } else if (key == "group") {
        _group_by = value;

      } else if (key == "const") {

        std::vector<std::string> const_str;
        boost::split(const_str, value, boost::is_any_of("."));

        _constraints[const_str[0]] = std::make_pair(const_str[1], const_str[2]);
      }
    }
  }
}
std::ostream &operator<<(std::ostream &os, const Query &query) {
  os << query._url;
  return os;
}

std::string Query::to_postgresql() const {
  std::string where;

  for (const auto &constraint : _constraints) {

    if (!where.empty()) {
      where += " AND ";
    }

    if (constraint.second.first == "tile") {
      auto clausule = Spatial::parse_tile(constraint.second.second);
      auto resolution = clausule.resolution;
      auto tile = clausule.tile;

      uint32_t curr_z = std::min((int32_t) resolution, 25 - (int32_t) tile.z);
      uint32_t n = (uint64_t) 1 << curr_z;

      uint32_t x_min = tile.x * n;
      uint32_t x_max = (tile.x + 1) * n;

      uint32_t y_min = tile.y * n;
      uint32_t y_max = (tile.y + 1) * n;

      curr_z += tile.z;

      std::string envelop;

      for (uint32_t x = x_min; x < x_max; ++x) {
        for (uint32_t y = y_min; y < y_max; ++y) {

          std::stringstream stream;

          std::string region_xmin = std::to_string(mercator_util::tilex2lon(x, curr_z));
          std::string region_xmax = std::to_string(mercator_util::tilex2lon(x + 1, curr_z));

          std::string region_ymin = std::to_string(mercator_util::tiley2lat(y + 1, curr_z));
          std::string region_ymax = std::to_string(mercator_util::tiley2lat(y, curr_z));

          if (!envelop.empty()) {
            envelop += " OR ";
          }

          envelop += "ST_MakeEnvelope(" + region_xmin + ", " + region_ymin +
              ", " + region_xmax + ", " + region_ymax + ")";
        }
      }

      where += constraint.first + " && (" + envelop + ")";

    } else if (constraint.second.first == "region") {

      auto region = Spatial::parse_region(constraint.second.second);

      std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
      std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
      std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
      std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

      where += constraint.first + " && ST_MakeEnvelope(" + region_xmin + ", " + region_ymin +
          ", " + region_xmax + ", " + region_ymax + ")";

    } else if (constraint.second.first == "values") {

      auto values = Categorical::parse_static(constraint.second.second);

      std::string clausule;
      for (const auto &v : values) {
        clausule += std::to_string(v) + ",";
      }

      where += constraint.first + " IN (" + clausule.substr(0, clausule.size() - 1) + ")";

    } else if (constraint.second.first == "interval") {

      auto interval = Temporal::parse_interval_static(constraint.second.second);

      where += constraint.first + " >= (SELECT TIMESTAMP 'epoch' + " + std::to_string(interval.bound[0])
          + " * INTERVAL '1 second') ";
      where += "AND " + constraint.first + " < (SELECT TIMESTAMP 'epoch' + " + std::to_string(interval.bound[1])
          + " * INTERVAL '1 second')";
    }
  }

  // get aggregation clausule
  auto &aggr = get_aggr().front();

  if (aggr.first == "count") {
    return "SELECT COUNT(*) from db WHERE " + where + ";";
  } else if (aggr.first == "quantile") {
    auto parameter = AgrrPDigest::get_parameters(aggr).front();

    return "select percentile_cont(" + std::to_string(parameter) + ") within group (order by " + aggr.second.first
        + ") from (SELECT * from db WHERE " + where + ") as foo;";
  } else {
    return std::string();
  }
}

std::string Query::to_sqlite() const {
  std::string where;

  for (const auto &constraint : _constraints) {

    if (!where.empty()) {
      where += " AND ";
    }

    if (constraint.second.first == "tile") {
      auto clausule = Spatial::parse_tile(constraint.second.second);
      auto resolution = clausule.resolution;
      auto tile = clausule.tile;

      uint32_t curr_z = std::min((int32_t) resolution, 25 - (int32_t) tile.z);
      uint32_t n = (uint64_t) 1 << curr_z;

      uint32_t x_min = tile.x * n;
      uint32_t x_max = (tile.x + 1) * n;

      uint32_t y_min = tile.y * n;
      uint32_t y_max = (tile.y + 1) * n;

      curr_z += tile.z;

      std::string envelop;

      for (uint32_t x = x_min; x < x_max; ++x) {
        for (uint32_t y = y_min; y < y_max; ++y) {

          std::stringstream stream;

          std::string region_xmin = std::to_string(mercator_util::tilex2lon(x, curr_z));
          std::string region_xmax = std::to_string(mercator_util::tilex2lon(x + 1, curr_z));

          std::string region_ymin = std::to_string(mercator_util::tiley2lat(y + 1, curr_z));
          std::string region_ymax = std::to_string(mercator_util::tiley2lat(y, curr_z));

          if (!envelop.empty()) {
            envelop += " OR ";
          }

          envelop += "MbrWithin(" + constraint.first + ", BuildMbr(";
          envelop += region_xmin + "," + region_ymin + ",";
          envelop += region_xmax + "," + region_ymax + ")) AND ROWID IN (";
          envelop += "SELECT pkid FROM idx_db_" + constraint.first + " WHERE ";
          envelop += "xmin >= " + region_xmin + " AND ";
          envelop += "xmax <= " + region_xmax + " AND ";
          envelop += "ymin >= " + region_ymin + " AND ";
          envelop += "ymax <= " + region_ymax + ")";
        }
      }

      where += constraint.first + " && (" + envelop + ")";

    } else if (constraint.second.first == "region") {

      auto region = Spatial::parse_region(constraint.second.second);

      std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
      std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
      std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
      std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

      where += "MbrWithin(" + constraint.first + ", BuildMbr(";
      where += region_xmin + "," + region_ymin + ",";
      where += region_xmax + "," + region_ymax + ")) AND ROWID IN (";
      where += "SELECT pkid FROM idx_db_" + constraint.first + " WHERE ";
      where += "xmin >= " + region_xmin + " AND ";
      where += "xmax <= " + region_xmax + " AND ";
      where += "ymin >= " + region_ymin + " AND ";
      where += "ymax <= " + region_ymax + ")";

    } else if (constraint.second.first == "values") {

      auto values = Categorical::parse_static(constraint.second.second);

      std::string clausule;
      for (const auto &v : values) {
        clausule += std::to_string(v) + ",";
      }

      where += constraint.first + " IN (" + clausule.substr(0, clausule.size() - 1) + ")";

    } else if (constraint.second.first == "interval") {

      auto interval = Temporal::parse_interval_static(constraint.second.second);

      where += constraint.first + " BETWEEN (SELECT datetime(" + std::to_string(interval.bound[0]) + ", 'unixepoch')) ";
      where += "AND (SELECT datetime(" + std::to_string(interval.bound[1]) + ", 'unixepoch'))";
    }
  }

  // get aggregation clausule
  auto &aggr = get_aggr().front();

  if (aggr.first == "count") {
    return "SELECT COUNT(*) from db WHERE " + where;
  } else if (aggr.first == "quantile") {
    auto parameter = AgrrPDigest::get_parameters(aggr).front();

    return "SELECT percentile(" + aggr.second.first + ", " + std::to_string(parameter * 100.f)
        + ") from db WHERE " + where;
  } else {
    return std::string();
  }
}

std::string Query::to_monetdb() const {
  std::string where;

  for (const auto &constraint : _constraints) {

    if (!where.empty()) {
      where += " AND ";
    }

    if (constraint.second.first == "tile") {
      auto clausule = Spatial::parse_tile(constraint.second.second);
      auto resolution = clausule.resolution;
      auto tile = clausule.tile;

      uint32_t curr_z = std::min((int32_t) resolution, 25 - (int32_t) tile.z);
      uint32_t n = (uint64_t) 1 << curr_z;

      uint32_t x_min = tile.x * n;
      uint32_t x_max = (tile.x + 1) * n;

      uint32_t y_min = tile.y * n;
      uint32_t y_max = (tile.y + 1) * n;

      curr_z += tile.z;

      std::string envelop;

      for (uint32_t x = x_min; x < x_max; ++x) {
        for (uint32_t y = y_min; y < y_max; ++y) {

          std::stringstream stream;

          std::string region_xmin = std::to_string(mercator_util::tilex2lon(x, curr_z));
          std::string region_xmax = std::to_string(mercator_util::tilex2lon(x + 1, curr_z));

          std::string region_ymin = std::to_string(mercator_util::tiley2lat(y + 1, curr_z));
          std::string region_ymax = std::to_string(mercator_util::tiley2lat(y, curr_z));

          if (!envelop.empty()) {
            envelop += " OR ";
          }

          envelop += "ST_Intersects(ST_MakeEnvelope(" + region_xmin + ", " + region_ymin +
              ", " + region_xmax + ", " + region_ymax + "), " + constraint.first + ")";
        }
      }

      where += constraint.first + " && (" + envelop + ")";

    } else if (constraint.second.first == "region") {

      auto region = Spatial::parse_region(constraint.second.second);

      std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
      std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
      std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
      std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

      where += "ST_Intersects(ST_MakeEnvelope(" + region_xmin + ", " + region_ymin +
          ", " + region_xmax + ", " + region_ymax + "), " + constraint.first + ")";

    } else if (constraint.second.first == "values") {

      auto values = Categorical::parse_static(constraint.second.second);

      std::string clausule;
      for (const auto &v : values) {
        clausule += std::to_string(v) + ",";
      }

      where += constraint.first + " IN (" + clausule.substr(0, clausule.size() - 1) + ")";

    } else if (constraint.second.first == "interval") {

      auto interval = Temporal::parse_interval_static(constraint.second.second);

      where +=
          constraint.first + " >= (SELECT sys.str_to_timestamp('" + std::to_string(interval.bound[0]) + "', '%s')) ";
      where += "AND " + constraint.first + " < (SELECT sys.str_to_timestamp('" + std::to_string(interval.bound[1])
          + "', '%s'))";
    }
  }

  // get aggregation clausule
  auto &aggr = get_aggr().front();

  if (aggr.first == "count") {
    return "SELECT COUNT(*) from db WHERE " + where + ";";
  } else if (aggr.first == "quantile") {
    auto parameter = AgrrPDigest::get_parameters(aggr).front();
    return "SELECT quantile(" + aggr.second.first + ", " + std::to_string(parameter) + ") from db WHERE " + where + ";";
  } else {
    return std::string();
  }
}
