#include "stdafx.h"
#include "types.h"
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

void MonetDBCtn::create() {
  update(_dbh, "DROP TABLE IF EXISTS db");
  update(_dbh,
         "CREATE TABLE db (user_id INTEGER, time TIMESTAMP, hour_of_day INTEGER, day_of_week INTEGER, coord POINT)");
}

void MonetDBCtn::insert(const std::string &filename) {

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

      // (user_id INTEGER, time DATETIME, hour_of_day INTEGER, day_of_week INTEGER, coord POINT)

      std::string sql = "INSERT INTO db VALUES ('" + data[0] + "', '" + data[1] + "', " + data[5] + ", " + data[6]
          + ", 'POINT( " + data[3] + " " + data[2] + " )')";

      update(_dbh, (char *) sql.c_str());

    } catch (const std::exception &e) {
      std::cerr << "[" << e.what() << "]: [" << line << "]" << std::endl;
    }
  }

  infile.close();











  /*char *name;
  char *age;

  update(_dbh, "CREATE TABLE emp (name VARCHAR(20), age INT)");
  update(_dbh, "INSERT INTO emp VALUES ('John', 23)");
  update(_dbh, "INSERT INTO emp VALUES ('Mary', 22)");
  MapiHdl hdl = query(_dbh, "SELECT * FROM emp");

  while (mapi_fetch_row(hdl)) {
    name = mapi_fetch_field(hdl, 0);
    age = mapi_fetch_field(hdl, 1);
    printf("%s is %s\n", name, age);
  }

  mapi_close_handle(hdl);*/

}

void MonetDBCtn::query(const Query &query) {

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

MapiHdl MonetDBCtn::query(Mapi dbh, char *q) {
  MapiHdl ret = NULL;

  if ((ret = mapi_query(dbh, q)) == NULL || mapi_error(dbh) != MOK)
    die(dbh, ret);

  return (ret);
}

void MonetDBCtn::update(Mapi dbh, char *q) {
  MapiHdl ret = query(dbh, q);

  if (mapi_close_handle(ret) != MOK)
    die(dbh, ret);
}