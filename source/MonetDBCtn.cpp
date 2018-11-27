#include "stdafx.h"
#include "types.h"
#include "MonetDBCtn.h"


MonetDBCtn::MonetDBCtn(int argc, char **argv) {
  Mapi dbh;
  MapiHdl hdl = NULL;
  char *name;
  char *age;
  dbh = mapi_connect("localhost", 50000, "monetdb", "monetdb", "sql", "db");

  if (mapi_error(dbh))
    die(dbh, hdl);

  update(dbh, "CREATE TABLE emp (name VARCHAR(20), age INT)");
  update(dbh, "INSERT INTO emp VALUES ('John', 23)");
  update(dbh, "INSERT INTO emp VALUES ('Mary', 22)");
  hdl = query(dbh, "SELECT * FROM emp");

  while (mapi_fetch_row(hdl)) {
    name = mapi_fetch_field(hdl, 0);
    age = mapi_fetch_field(hdl, 1);
    printf("%s is %s\n", name, age);
  }

  mapi_close_handle(hdl);
  mapi_destroy(dbh);

}
MonetDBCtn::~MonetDBCtn() {

}
void MonetDBCtn::create() {

}
void MonetDBCtn::insert(const std::string &filename) {

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