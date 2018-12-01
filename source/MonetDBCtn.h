#pragma once
#include "GeoCtnIntf.h"
#include <monetdb/mapi.h>

class MonetDBCtn : public GeoCtnIntf {
 public:
  MonetDBCtn(int argc, char *argv[]);

  ~MonetDBCtn() override;

  // snap
  void create_snap() override;
  void insert_snap(const std::string &filename) override;

  // on-time
  void create_on_time() override;
  void insert_on_time(const std::string &filename) override;

  // small-twitter
  void create_small_twitter() override;
  void insert_small_twitter(const std::string &filename) override;

  void query(const Query &query) override;

  inline virtual std::string name() const override;

  static void die(Mapi dbh, MapiHdl hdl);
  static MapiHdl query_db(Mapi dbh, char *q);
  static void update(Mapi dbh, char *q);
  static void check(Mapi dbh, MapiHdl hdl, int ret = MOK, bool fatal = true);

 private:
  bool _init{false};
#ifdef __GNUC__
  Mapi _dbh;
#endif // __GNUC__
};

std::string MonetDBCtn::name() const {
  static auto name_str = "MonetDB";
  return name_str;
}
