#include "stdafx.h"
#include "MonetDBCtn.h"

MonetDBCtn::MonetDBCtn(int argc, char **argv) {
#ifdef __GNUC__
  MapiHdl hdl = NULL;
  _dbh = mapi_connect("localhost", 50000, "monetdb", "monetdb", "sql", "db");

  if (mapi_error(_dbh)) {
    die(_dbh, hdl);
  } else {
    _init = true;
  }
#endif // __GNUC__
}

MonetDBCtn::~MonetDBCtn() {
#ifdef __GNUC__
  mapi_destroy(_dbh);
#endif // __GNUC__
}

void MonetDBCtn::create_snap() {
#ifdef __GNUC__
  update(_dbh, (char *) "DROP TABLE IF EXISTS db");
  update(_dbh,
         (char *) "CREATE TABLE db (user_id INTEGER, time TIMESTAMP, hour_of_day INTEGER, day_of_week INTEGER, coord POINT)"
  );
#endif // __GNUC__
}

void MonetDBCtn::insert_snap(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
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

  MapiHdl hdl = mapi_prepare(_dbh, "INSERT INTO db VALUES (?, ?, ?, ?, ?)");
  check(_dbh, hdl);

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

      // (user_id INTEGER, time DATETIME, hour_of_day INTEGER, day_of_week INTEGER, coord POINT)

      mapi_param(hdl, 0, (char **) data[0].c_str());

      mapi_param(hdl, 2, (char **) data[5].c_str());
      mapi_param(hdl, 3, (char **) data[6].c_str());

      std::string date = "(SELECT sys.str_to_timestamp('" + data[1] + "', '%Y-%m-%dT%H:%M:%SZ'))";
      std::string coord = "'POINT( " + data[3] + " " + data[2] + " )'";

      mapi_param(hdl, 1, (char **) date.c_str());
      mapi_param(hdl, 4, (char **) coord.c_str());

      int ret = mapi_execute(hdl);
      check(_dbh, hdl, ret);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  // update bounding box
  update(_dbh, (char *) "ALTER TABLE db ADD bbox mbr");
  update(_dbh, (char *) "UPDATE db SET bbox = mbr(coord)");

#endif // __GNUC__
}

void MonetDBCtn::create_on_time() {
#ifdef __GNUC__
  update(_dbh, (char *) "DROP TABLE IF EXISTS db");
  update(_dbh,
         (char *) "CREATE TABLE db (on_time INTEGER, unique_carrier INTEGER, crs_dep_time TIMESTAMP, origin_airport POINT, dep_delay_t REAL)"
  );
#endif // __GNUC__
}

void MonetDBCtn::insert_on_time(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
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

  MapiHdl hdl = mapi_prepare(_dbh, "INSERT INTO db VALUES (?, ?, ?, ?, ?)");
  check(_dbh, hdl);

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

      // (char *) "CREATE TABLE db (on_time INTEGER, unique_carrier INTEGER, crs_dep_time TIMESTAMP, origin_airport POINT, dep_delay_t REAL)"
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

      const long epoch = std::stoul(data[7]);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&epoch), "%FT%TZ");
      auto crs_dep_time = ss.str();

      std::string date = "(SELECT sys.str_to_timestamp('" + crs_dep_time + "', '%Y-%m-%dT%H:%M:%SZ'))";

      mapi_param(hdl, 0, (char **) data[1].c_str());
      mapi_param(hdl, 1, (char **) data[4].c_str());
      mapi_param(hdl, 2, (char **) date.c_str());

      std::string coord = "'POINT( " + data[3] + " " + data[2] + " )'";
      mapi_param(hdl, 3, (char **) coord.c_str());

      mapi_param(hdl, 4, (char **) data[13].c_str());

      int ret = mapi_execute(hdl);
      check(_dbh, hdl, ret);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  // update bounding box
  update(_dbh, (char *) "ALTER TABLE db ADD bbox mbr");
  update(_dbh, (char *) "UPDATE db SET bbox = mbr(origin_airport)");

#endif // __GNUC__
}

void MonetDBCtn::create_small_twitter() {
#ifdef __GNUC__
  update(_dbh, (char *) "DROP TABLE IF EXISTS db");
  update(_dbh, (char *) "CREATE TABLE db (device INTEGER, time TIMESTAMP, coord POINT)");
#endif // __GNUC__
}

void MonetDBCtn::insert_small_twitter(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
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

  MapiHdl hdl = mapi_prepare(_dbh, "INSERT INTO db VALUES (?, ?, ?)");
  check(_dbh, hdl);

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

      // update(_dbh, (char *) "CREATE TABLE db (device INTEGER, time TIMESTAMP, coord POINT)");
      /*
      00, app
      01, device
      02, language
      03, time
      04, coord
      05, coord
      */

      const long epoch = std::stoul(data[3]);
      std::stringstream ss;
      ss << std::put_time(std::localtime(&epoch), "%FT%TZ");
      auto time = ss.str();

      std::string date = "(SELECT sys.str_to_timestamp('" + time + "', '%Y-%m-%dT%H:%M:%SZ'))";

      mapi_param(hdl, 0, (char **) data[1].c_str());
      mapi_param(hdl, 1, (char **) date.c_str());

      std::string coord = "'POINT( " + data[5] + " " + data[4] + " )'";
      mapi_param(hdl, 2, (char **) coord.c_str());

      int ret = mapi_execute(hdl);
      check(_dbh, hdl, ret);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  // update bounding box
  update(_dbh, (char *) "ALTER TABLE db ADD bbox mbr");
  update(_dbh, (char *) "UPDATE db SET bbox = mbr(coord)");

#endif // __GNUC__
}

void MonetDBCtn::create_gaussian() {
#ifdef __GNUC__
  update(_dbh, (char *) "DROP TABLE IF EXISTS db");
  update(_dbh, (char *) "CREATE TABLE db (gaussian REAL, coord POINT)");
#endif // __GNUC__
}

void MonetDBCtn::insert_gaussian(const std::string &filename) {
#ifdef __GNUC__

  if (!_init) {
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

  MapiHdl hdl = mapi_prepare(_dbh, "INSERT INTO db VALUES (?, ?)");
  check(_dbh, hdl);

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

      // update(_dbh, (char *) "CREATE TABLE db (gaussian REAL, coord POINT)");
      /*
      00, x
      01, y
      02, z
      03, t
      */

      mapi_param(hdl, 0, (char **) data[2].c_str());

      std::string coord = "'POINT( " + data[0] + " " + data[1] + " )'";
      mapi_param(hdl, 1, (char **) coord.c_str());

      int ret = mapi_execute(hdl);
      check(_dbh, hdl, ret);

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();

  // update bounding box
  update(_dbh, (char *) "ALTER TABLE db ADD bbox mbr");
  update(_dbh, (char *) "UPDATE db SET bbox = mbr(coord)");

#endif // __GNUC__
}

void MonetDBCtn::query(const Query &query) {
#ifdef __GNUC__
  TIMER_DECLARE
  TIMER_START

  if (!_init) {
    TIMER_END
    TIMER_OUTPUT(name())
    return;
  }

  // mapi_export char *mapi_fetch_field(MapiHdl hdl, int fnr);
  // mapi_export size_t mapi_fetch_field_len(MapiHdl hdl, int fnr);

  try {
    MapiHdl hdl = query_db(_dbh, (char *) query.to_monetdb().c_str());

    if (mapi_fetch_row(hdl)) {
      auto volatile size = mapi_fetch_field_len(hdl, 0);
      auto volatile field = mapi_fetch_field(hdl, 0);
    }

    mapi_close_handle(hdl);
  } catch (const std::exception &e) {
    std::cerr << "[" << e.what() << "]: [" << query.to_monetdb() << "]" << std::endl;
  }

  TIMER_END
  TIMER_OUTPUT(name(), "output")

#endif // __GNUC__
}

void MonetDBCtn::check(Mapi dbh, MapiHdl hdl, int ret, bool fatal) {
  if (ret != MOK || hdl == NULL || mapi_error(dbh)) {
    if (hdl) {
      mapi_explain_result(hdl, stderr);
    } else {
      mapi_explain(dbh, stderr);
    }
    if (fatal) assert(0);//exit(-1);
  }
}

void MonetDBCtn::die(Mapi dbh, MapiHdl hdl) {
  if (hdl != NULL) {
    mapi_explain_query(hdl, stderr);
    do {
      if (mapi_result_error(hdl) != NULL)
        mapi_explain_result(hdl, stderr);
    } while (mapi_next_result(hdl) == 1);
    mapi_close_handle(hdl);
    mapi_destroy(dbh);
  } else if (dbh != NULL) {
    mapi_explain(dbh, stderr);
    mapi_destroy(dbh);
  } else {
    fprintf(stderr, "command failed\n");
  }
  exit(-1);
}

MapiHdl MonetDBCtn::query_db(Mapi dbh, char *q) {
  MapiHdl ret = NULL;

  if ((ret = mapi_query(dbh, q)) == NULL || mapi_error(dbh) != MOK)
    die(dbh, ret);

  return (ret);
}

void MonetDBCtn::update(Mapi dbh, char *q) {
  MapiHdl ret = query_db(dbh, q);

  if (mapi_close_handle(ret) != MOK)
    die(dbh, ret);
}
