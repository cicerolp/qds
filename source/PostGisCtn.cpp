#include "stdafx.h"
#include "types.h"
#include "PostGisCtn.h"

PostGisCtn::PostGisCtn(int argc, char *argv[]) {
#ifdef __GNUC__
  std::string conninfo = "user=postgres host=localhost port=5432 dbname=db";

  // make a connection to the database
  _conn = PQconnectdb(conninfo.c_str());

  // check to see that the backend connection was successfully made
  if (PQstatus(_conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(_conn));
  } else {
    _init = true;
  }
#endif // __GNUC__
}

PostGisCtn::~PostGisCtn() {
#ifdef __GNUC__
  // close the connection to the database and cleanup
  PQfinish(_conn);
#endif // __GNUC__
}

// build container
void PostGisCtn::create_snap() {
#ifdef __GNUC__

  PGresult *res;
  std::string sql;

  sql += "DROP TABLE IF EXISTS db;";
  sql +=
      "CREATE TABLE db(user_id INTEGER, time TIMESTAMP, hour_of_day INTEGER, day_of_week INTEGER, coord GEOMETRY(Point, 4326));";
  // spatial index using GIST
  sql += "CREATE INDEX key_gix ON db USING GIST(coord);";

  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql =
      "INSERT INTO db (user_id, time, hour_of_day, day_of_week, coord) VALUES ($1, $2, $3, $4, ST_GeomFromText($5, 4326));";
  res = PQprepare(_conn, "stmtname", sql.c_str(), 0, nullptr);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PQprepare command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  _init = true;

#endif // __GNUC__

  return;
}

// update container
void PostGisCtn::insert_snap(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
    return;
  }

  PGresult *res;
  std::string sql;

  sql = "BEGIN;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  int paramLengths[5];
  int paramFormats[5] = {0, 0, 0, 0, 0};
  const char *paramValues[5];

  static const std::string sep = ",";

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/
  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(sep + "(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  std::ifstream infile(filename);

  std::string line;

  // skip header
  std::getline(infile, line);

  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    try {
      // split line to tokens
      boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
      boost::sregex_token_iterator ti_end;

      std::vector<std::string> data(ti, ti_end);

      // user_id -> data[0]
      // date -> data[1]
      // hour_of_day -> data[5]
      // day_of_week -> data[6]

      // coord[lon, lat] -> data[3, 2]
      char coord[50];
      sprintf(coord, "POINT(%f %f)", std::stof(data[3]), std::stof(data[2]));

      paramValues[0] = data[0].c_str();
      paramValues[1] = data[1].c_str();
      paramValues[2] = data[5].c_str();
      paramValues[3] = data[6].c_str();
      paramValues[4] = coord;

      paramLengths[0] = std::strlen(paramValues[0]);
      paramLengths[1] = std::strlen(paramValues[1]);
      paramLengths[2] = std::strlen(paramValues[2]);
      paramLengths[3] = std::strlen(paramValues[3]);
      paramLengths[4] = std::strlen(paramValues[4]);

      res = PQexecPrepared(_conn, "stmtname", 5, paramValues, paramLengths, paramFormats, 0);
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "PQexecPrepared command failed: %s", PQerrorMessage(_conn));
      }
      PQclear(res);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  sql = "COMMIT;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "COMMIT command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  // reorders the table on disk based on the index
  sql = "CLUSTER db USING key_gix;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "CLUSTER command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql = "ANALYZE db;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "ANALYZE command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

#endif // __GNUC__
}

void PostGisCtn::create_on_time() {
#ifdef __GNUC__

  PGresult *res;
  std::string sql;

  sql += "DROP TABLE IF EXISTS db;";
  sql +=
      "CREATE TABLE db(on_time INTEGER, unique_carrier INTEGER, crs_dep_time TIMESTAMP, origin_airport GEOMETRY(Point, 4326), dep_delay_t REAL);";
  // spatial index using GIST
  sql += "CREATE INDEX key_gix ON db USING GIST(origin_airport);";

  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql =
      "INSERT INTO db (on_time, unique_carrier, crs_dep_time, origin_airport, dep_delay_t) VALUES ($1, $2, $3, ST_GeomFromText($4, 4326), $5);";
  res = PQprepare(_conn, "stmtname", sql.c_str(), 0, nullptr);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PQprepare command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  _init = true;

#endif // __GNUC__

  return;
}

void PostGisCtn::insert_on_time(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
    return;
  }

  PGresult *res;
  std::string sql;

  sql = "BEGIN;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  int paramLengths[5];
  int paramFormats[5] = {0, 0, 0, 0, 0};
  const char *paramValues[5];

  static const std::string sep = ",";

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/
  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(sep + "(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  std::ifstream infile(filename);

  std::string line;

  // skip header
  std::getline(infile, line);

  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    try {
      // split line to tokens
      boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
      boost::sregex_token_iterator ti_end;

      std::vector<std::string> data(ti, ti_end);

      // "INSERT INTO db (on_time, unique_carrier, crs_dep_time, origin_airport, dep_delay_t) VALUES ($1, $2, $3, ST_GeomFromText($4, 4326), $5);";
      /*
      00, arr_on_time
      01, dep_on_time
      02, cancelled
      03, diverted
      04, unique_carrier
      05, origin_airport_id
      06, dest_airport_id
      07, crs_dep_time
      08, crs_arr_time
      09, origin_airport
      10, origin_airport
      11, dest_airport
      12, dest_airport
      13, dep_delay
      14, dep_delay_minutes
      15, arr_delay
      16, arr_delay_minutes
      17, actual_elapsed_time
      18, air_time
      19, distance
      */

      // coord[lon, lat] -> data[3, 2]
      char coord[50];
      sprintf(coord, "POINT(%f %f)", std::stof(data[10]), std::stof(data[9]));

      paramValues[0] = data[1].c_str();
      paramValues[1] = data[4].c_str();

      const long epoch = std::stoul(data[7]);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&epoch), "%FT%TZ");
      auto crs_dep_time = ss.str();

      paramValues[2] = crs_dep_time.c_str();
      paramValues[3] = coord;
      paramValues[4] = data[13].c_str();

      paramLengths[0] = std::strlen(paramValues[0]);
      paramLengths[1] = std::strlen(paramValues[1]);
      paramLengths[2] = std::strlen(paramValues[2]);
      paramLengths[3] = std::strlen(paramValues[3]);
      paramLengths[4] = std::strlen(paramValues[4]);

      res = PQexecPrepared(_conn, "stmtname", 5, paramValues, paramLengths, paramFormats, 0);
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "PQexecPrepared command failed: %s", PQerrorMessage(_conn));
      }
      PQclear(res);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  sql = "COMMIT;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "COMMIT command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  // reorders the table on disk based on the index
  sql = "CLUSTER db USING key_gix;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "CLUSTER command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql = "ANALYZE db;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "ANALYZE command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

#endif // __GNUC__
}

void PostGisCtn::create_small_twitter() {
#ifdef __GNUC__

  PGresult *res;
  std::string sql;

  sql += "DROP TABLE IF EXISTS db;";
  sql +=
      "CREATE TABLE db(device INTEGER, time TIMESTAMP, coord GEOMETRY(Point, 4326));";
  // spatial index using GIST
  sql += "CREATE INDEX key_gix ON db USING GIST(coord);";

  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql =
      "INSERT INTO db (device, time, coord) VALUES ($1, $2, ST_GeomFromText($3, 4326));";
  res = PQprepare(_conn, "stmtname", sql.c_str(), 0, nullptr);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "PQprepare command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  _init = true;

#endif // __GNUC__

  return;
}

void PostGisCtn::insert_small_twitter(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
    return;
  }

  PGresult *res;
  std::string sql;

  sql = "BEGIN;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  int paramLengths[3];
  int paramFormats[3] = {0, 0, 0};
  const char *paramValues[5];

  static const std::string sep = ",";

  // source: https://bravenewmethod.com/2016/09/17/quick-and-robust-c-csv-reader-with-boost/
  // used to split the file in lines
  const boost::regex linesregx("\\r\\n|\\n\\r|\\n|\\r");

  // used to split each line to tokens, assuming ',' as column separator
  const boost::regex fieldsregx(sep + "(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))");

  std::ifstream infile(filename);

  std::string line;

  // skip header
  std::getline(infile, line);

  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    try {
      // split line to tokens
      boost::sregex_token_iterator ti(line.begin(), line.end(), fieldsregx, -1);
      boost::sregex_token_iterator ti_end;

      std::vector<std::string> data(ti, ti_end);
      /*
      00, app
      01, device
      02, language
      03, time
      04, coord
      05, coord
      */

      char coord[50];
      sprintf(coord, "POINT(%f %f)", std::stof(data[5]), std::stof(data[4]));

      const long epoch = std::stoul(data[3]);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&epoch), "%FT%TZ");
      auto time = ss.str();

      paramValues[0] = data[1].c_str();
      paramValues[1] = time.c_str();
      paramValues[2] = coord;

      paramLengths[0] = std::strlen(paramValues[0]);
      paramLengths[1] = std::strlen(paramValues[1]);
      paramLengths[2] = std::strlen(paramValues[2]);

      res = PQexecPrepared(_conn, "stmtname", 3, paramValues, paramLengths, paramFormats, 0);
      if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "PQexecPrepared command failed: %s", PQerrorMessage(_conn));
      }
      PQclear(res);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  sql = "COMMIT;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "COMMIT command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  // reorders the table on disk based on the index
  sql = "CLUSTER db USING key_gix;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "CLUSTER command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

  sql = "ANALYZE db;";
  res = PQexec(_conn, sql.c_str());
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "ANALYZE command failed: %s", PQerrorMessage(_conn));
  }
  PQclear(res);

#endif // __GNUC__
}

void PostGisCtn::query(const Query &query) {
  TIMER_DECLARE

#ifdef __GNUC__

  TIMER_START

  if (!_init) {
    TIMER_END
    TIMER_OUTPUT(name())
    return;
  }

  PGresult *res;
  std::string sql;

  res = PQexecParams(_conn, query.to_postgresql().c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "SELECT command failed: %s", PQerrorMessage(_conn));
  }

  // std::cout << query.to_postgresql() << std::endl;

  int count = -1;
  auto value = std::string(PQgetvalue(res, 0, 0));

  if (!value.empty()) {
    count = std::stoi(value);
  }

  PQclear(res);

  TIMER_END
  TIMER_OUTPUT(name(), "output", count)

#endif // __GNUC__
}

/*// apply function for every el<valuetype>
duration_t PostGisCtn::scan_at_region(const region_t& region, scantype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  if (!_init) {
    return {duration_info("scan_at_region", timer)};
  }

  timer.start();

  PGresult* res;
  std::string sql;

  std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
  std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
  std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
  std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

  sql = "SELECT value from db Where key && ST_MakeEnvelope(" + region_xmin + ", " + region_ymin + ", " + region_xmax + ", " + region_ymax + ");";

  res = PQexecParams(_conn, sql.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "SELECT command failed: %s", PQerrorMessage(_conn));
  }

  uint32_t n_tuples = PQntuples(res);
  for (uint32_t row = 0; row < n_tuples; ++row) {
    __apply((*(valuetype*)PQgetvalue(res, row, 0)));
  }

  PQclear(res);

  timer.stop();

#endif // __GNUC__

  return {duration_info("scan_at_region", timer)};
}

// apply function for every spatial area/region
duration_t PostGisCtn::apply_at_tile(const region_t& region, applytype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  if (!_init) {
    return {duration_info("apply_at_tile", timer)};
  }

  timer.start();

  PGresult* res;
  std::string sql;

  uint32_t curr_z = std::min((uint32_t)8, 25 - region.z);
  uint32_t n = (uint64_t)1 << curr_z;

  uint32_t x_min = region.x0 * n;
  uint32_t x_max = (region.x1 + 1) * n;

  uint32_t y_min = region.y0 * n;
  uint32_t y_max = (region.y1 + 1) * n;

  curr_z += region.z;

  for (uint32_t x = x_min; x < x_max; ++x) {
    for (uint32_t y = y_min; y < y_max; ++y) {

      std::stringstream stream;

      std::string xmin = std::to_string(mercator_util::tilex2lon(x, curr_z));
      std::string xmax = std::to_string(mercator_util::tilex2lon(x + 1, curr_z));

      std::string ymin = std::to_string(mercator_util::tiley2lat(y + 1, curr_z));
      std::string ymax = std::to_string(mercator_util::tiley2lat(y, curr_z));


      sql = "SELECT count(*) from db Where key && ST_MakeEnvelope(" + xmin + ", " + xmax + ", " + ymin + ", " + ymax + ");";

      res = PQexecParams(_conn, sql.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 0);
      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT command failed: %s", PQerrorMessage(_conn));
      }

      uint32_t count = std::stoi(PQgetvalue(res, 0, 0));
      if (count > 0) __apply(spatial_t(x, y, curr_z), count);

      PQclear(res);
    }
  }

  timer.stop();

#endif // __GNUC__

  return {duration_info("apply_at_tile", timer)};
}

duration_t PostGisCtn::apply_at_region(const region_t& region, applytype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  if (!_init) {
    return {duration_info("apply_at_region", timer)};
  }

  timer.start();

  PGresult* res;
  std::string sql;

  std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
  std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
  std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
  std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

  sql = "SELECT count(*) from db Where key && ST_MakeEnvelope(" + region_xmin + ", " + region_ymin + ", " + region_xmax + ", " + region_ymax + ");";

  res = PQexecParams(_conn, sql.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 0);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    fprintf(stderr, "SELECT command failed: %s", PQerrorMessage(_conn));
  }

  uint32_t count = std::stoi(PQgetvalue(res, 0, 0));
  if (count > 0) {
    __apply(spatial_t(region.x0 + (uint32_t)((region.x1 - region.x0) / 2),
                      region.y0 + (uint32_t)((region.y1 - region.y0) / 2),
                      0), count);
  }

  PQclear(res);

  timer.stop();

#endif // __GNUC__

  return {duration_info("apply_at_region", timer)};
}*/
