#include "stdafx.h"
#include "types.h"
#include "SpatiaLiteCtn.h"

SpatiaLiteCtn::SpatiaLiteCtn(int argc, char *argv[]) {
#ifdef __GNUC__
  int ret;

  std::string db = ":memory:";
  // std::string db = "db.sqlite";

  // in-memory database
  ret = sqlite3_open_v2(db.c_str(), &_handle,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (ret != SQLITE_OK) {
    printf("cannot open '%s': %s\n", db.c_str(), sqlite3_errmsg(_handle));
    sqlite3_close(_handle);
    return;
  }
  _cache = spatialite_alloc_connection();
  spatialite_init_ex(_handle, _cache, 0);

  printf("SQLite version: %s\n", sqlite3_libversion());

  printf("SpatiaLite version: %s\n", spatialite_version());

  initGEOS(notice, log_and_exit);
  printf("GEOS version %s\n", GEOSversion());

  printf("\n\n");
#endif // __GNUC__
}

SpatiaLiteCtn::~SpatiaLiteCtn() {
#ifdef __GNUC__
  finishGEOS();

  if (_handle) sqlite3_close(_handle);
  if (_cache) spatialite_cleanup_ex(_cache);

  //spatialite_shutdown();
#endif // __GNUC__
}

// build container
void SpatiaLiteCtn::create() {
  Timer timer;

#ifdef __GNUC__

  int ret;
  char sql[256];
  char *err_msg = NULL;

  timer.start();

  // we are supposing this one is an empty database,
  // so we have to create the Spatial Metadata
  strcpy(sql, "SELECT InitSpatialMetadata(1)");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("InitSpatialMetadata() error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // now we can create the table
  // for simplicity we'll define only one column, the primary key
  strcpy(sql, "CREATE TABLE db (");
  strcat(sql, "user_id INTEGER, time DATETIME, hour_of_day INTEGER, day_of_week INTEGER)");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("CREATE TABLE 'db' error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // ... we'll add a Geometry column of POINT type to the table
  strcpy(sql, "SELECT AddGeometryColumn('db', 'coord', 4326, 'POINT', 2)");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("AddGeometryColumn() error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // and finally we'll enable this geo-column to have a Spatial Index based on R*Tree
  strcpy(sql, "SELECT CreateSpatialIndex('db', 'coord')");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("CreateSpatialIndex() error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  _init = true;
  timer.stop();

#endif // __GNUC__
}

// update container
void SpatiaLiteCtn::insert(const std::string &filename) {
  duration_t duration;

#ifdef __GNUC__

  Timer timer;

  if (!_init) {
    return;
  }

  // insert start
  timer.start();

  int ret;
  char sql[256];
  char *err_msg = NULL;

  int blob_size;
  unsigned char *blob;

  sqlite3_stmt *stmt;
  gaiaGeomCollPtr geo = NULL;

  // beginning a transaction
  strcpy(sql, "BEGIN");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("BEGIN error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // preparing to populate the table
  strcpy(sql, "INSERT INTO db (user_id, time, hour_of_day, day_of_week, coord) VALUES (?, ?, ?, ?, ?)");
  ret = sqlite3_prepare_v2(_handle, sql, strlen(sql), &stmt, NULL);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("INSERT SQL error: %s\n", sqlite3_errmsg(_handle));

    timer.stop();
    return;
  }

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

      // preparing the geometry to insert
      geo = gaiaAllocGeomColl();
      geo->Srid = 4326;
      // lon, lat
      gaiaAddPointToGeomColl(geo, std::stof(data[3]), std::stof(data[2]));

      // transforming this geometry into the SpatiaLite BLOB format
      gaiaToSpatiaLiteBlobWkb(geo, &blob, &blob_size);

      // we can now destroy the geometry object
      gaiaFreeGeomColl(geo);

      // resetting Prepared Statement and bindings
      sqlite3_reset(stmt);
      sqlite3_clear_bindings(stmt);

      // user_id, time, hour_of_day, day_of_week, coord

      // (pk, key, value)
      // binding parameters to Prepared Statement
      sqlite3_bind_int(stmt, 1, std::stoi(data[0]));
      sqlite3_bind_text(stmt, 2, data[1].c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(stmt, 3, std::stoi(data[5]));
      sqlite3_bind_int(stmt, 4, std::stoi(data[6]));
      sqlite3_bind_blob(stmt, 5, blob, blob_size, free);

      // performing actual row insert
      ret = sqlite3_step(stmt);
      if (ret == SQLITE_DONE || ret == SQLITE_ROW);
      else {
        // an error occurred
        printf("sqlite3_step() error: %s\n", sqlite3_errmsg(_handle));
        sqlite3_finalize(stmt);

        timer.stop();
        return;
      }

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  // we have now to finalize the query [memory cleanup]
  sqlite3_finalize(stmt);

  // committing the transaction
  strcpy(sql, "COMMIT");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("COMMIT error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // insert end
  timer.stop();
  duration.emplace_back("Insert", timer);

  // analyze start
  timer.start();

  // now we'll optimize the table
  strcpy(sql, "ANALYZE db");
  ret = sqlite3_exec(_handle, sql, NULL, NULL, &err_msg);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("ANALYZE error: %s\n", err_msg);
    sqlite3_free(err_msg);

    timer.stop();
    return;
  }

  // analyze end
  timer.stop();
  duration.emplace_back("Analyze", timer);

#endif // __GNUC__
}

void SpatiaLiteCtn::query(const Query &query) {
  TIMER_DECLARE

#ifdef __GNUC__

  TIMER_START

  if (!_init) {
    TIMER_END
    TIMER_OUTPUT(name())
    return;
  }

  int ret;

  sqlite3_stmt *stmt;

  // preparing to populate the table
  ret = sqlite3_prepare_v2(_handle, query.to_sqlite().c_str(), query.to_sqlite().size(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("SELECT SQL error: %s\n", sqlite3_errmsg(_handle));

    TIMER_END
    TIMER_OUTPUT(name())
    return;
  }

  sqlite3_finalize(stmt);

  TIMER_END
  TIMER_OUTPUT(name())

#endif // __GNUC__
}

/*// apply function for every el<valuetype>
duration_t SpatiaLiteCtn::scan_at_region(const region_t& region, scantype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  timer.start();

  if (!_init) {
    timer.stop();
    return {duration_info("total", timer)};
  }

  int ret;
  char sql[256];
  char* err_msg = NULL;

  std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
  std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
  std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
  std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

  std::stringstream stream;
  stream << "SELECT value FROM db ";
  stream << "WHERE MbrWithin(key, BuildMbr(";
  stream << region_xmin << "," << region_ymin << ",";
  stream << region_xmax << "," << region_ymax << ")) AND ROWID IN (";
  stream << "SELECT pkid FROM idx_db_key WHERE ";
  stream << "xmin >= " << region_xmin << " AND ";
  stream << "xmax <= " << region_xmax << " AND ";
  stream << "ymin >= " << region_ymin << " AND ";
  stream << "ymax <= " << region_ymax << ")";

  sqlite3_stmt* stmt;

  // preparing to populate the table
  ret = sqlite3_prepare_v2(_handle, stream.str().c_str(), stream.str().size(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("INSERT SQL error: %s\n", sqlite3_errmsg(_handle));

    timer.stop();
    return {duration_info("total", timer)};
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    __apply((*(valuetype*)sqlite3_column_blob(stmt, 0)));
  }

  sqlite3_finalize(stmt);

  timer.stop();

#endif // __GNUC__

  return {duration_info("total", timer)};
}

// apply function for every spatial area/region
duration_t SpatiaLiteCtn::apply_at_tile(const region_t& region, applytype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  timer.start();

  if (!_init) {
    timer.stop();
    return {duration_info("total", timer)};
  }

  int ret;
  char sql[256];
  char* err_msg = NULL;

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

      stream << "SELECT count(*) FROM db ";
      stream << "WHERE ST_WITHIN(key, BuildMbr(";
      stream << xmin << "," << ymin << ",";
      stream << xmax << "," << ymax << ")) AND ROWID IN (";
      stream << "SELECT pkid FROM idx_db_key WHERE ";
      stream << "xmin >= " << xmin << " AND ";
      stream << "xmax <= " << xmax << " AND ";
      stream << "ymin >= " << ymin << " AND ";
      stream << "ymax <= " << ymax << ")";

      sqlite3_stmt* stmt;

      // preparing to populate the table
      ret = sqlite3_prepare_v2(_handle, stream.str().c_str(), stream.str().size(), &stmt, NULL);
      if (ret != SQLITE_OK) {
        // an error occurred
        printf("INSERT SQL error: %s\n", sqlite3_errmsg(_handle));

        timer.stop();
        return {duration_info("total", timer)};
      }

      if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        if (count > 0) __apply(spatial_t(x, y, curr_z), count);
      }

      sqlite3_finalize(stmt);
    }
  }

  timer.stop();

#endif // __GNUC__

  return {duration_info("total", timer)};
}

duration_t SpatiaLiteCtn::apply_at_region(const region_t& region, applytype_function __apply) {
  Timer timer;

#ifdef __GNUC__

  timer.start();

  if (!_init) {
    timer.stop();
    return {duration_info("total", timer)};
  }

  int ret;
  char sql[256];
  char* err_msg = NULL;

  std::string region_xmin = std::to_string(mercator_util::tilex2lon(region.x0, region.z));
  std::string region_xmax = std::to_string(mercator_util::tilex2lon(region.x1 + 1, region.z));
  std::string region_ymin = std::to_string(mercator_util::tiley2lat(region.y1 + 1, region.z));
  std::string region_ymax = std::to_string(mercator_util::tiley2lat(region.y0, region.z));

  std::stringstream stream;
  stream << "SELECT count(*) FROM db ";
  stream << "WHERE MbrWithin(key, BuildMbr(";
  stream << region_xmin << "," << region_ymin << ",";
  stream << region_xmax << "," << region_ymax << ")) AND ROWID IN (";
  stream << "SELECT pkid FROM idx_db_key WHERE ";
  stream << "xmin >= " << region_xmin << " AND ";
  stream << "xmax <= " << region_xmax << " AND ";
  stream << "ymin >= " << region_ymin << " AND ";
  stream << "ymax <= " << region_ymax << ")";

  sqlite3_stmt* stmt;

  // preparing to populate the table
  ret = sqlite3_prepare_v2(_handle, stream.str().c_str(), stream.str().size(), &stmt, NULL);
  if (ret != SQLITE_OK) {
    // an error occurred
    printf("INSERT SQL error: %s\n", sqlite3_errmsg(_handle));

    timer.stop();
    return {duration_info("total", timer)};
  }

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    int count = sqlite3_column_int(stmt, 0);
    if (count > 0) {
      __apply(spatial_t(region.x0 + (uint32_t)((region.x1 - region.x0) / 2),
                        region.y0 + (uint32_t)((region.y1 - region.y0) / 2),
                        0), count);
    }
  }

  sqlite3_finalize(stmt);

  timer.stop();

#endif // __GNUC__

  return {duration_info("total", timer)};
}*/

void SpatiaLiteCtn::notice(const char *fmt, ...) {
#ifdef __GNUC__
  va_list ap;

  fprintf(stdout, "NOTICE: ");

  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n");
#endif // __GNUC__
}

void SpatiaLiteCtn::log_and_exit(const char *fmt, ...) {
#ifdef __GNUC__
  va_list ap;

  fprintf(stdout, "ERROR: ");

  va_start(ap, fmt);
  vfprintf(stdout, fmt, ap);
  va_end(ap);
  fprintf(stdout, "\n");
  exit(1);
#endif // __GNUC__
}

