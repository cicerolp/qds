#pragma once
#include "GeoCtnIntf.h"
#include <monetdb/mapi.h>

class MonetDBCtn : public GeoCtnIntf {
 public:
  MonetDBCtn(int argc, char *argv[]);

  ~MonetDBCtn() override;

  // build container
  void create() override;

  // update container
  void insert(const std::string &filename) override;

  void query(const Query &query) override;

  inline virtual std::string name() const override;

  static void die(Mapi dbh, MapiHdl hdl);
  static MapiHdl query(Mapi dbh, char *q);
  static void update(Mapi dbh, char *q);

 private:
  bool _init{false};
#ifdef __GNUC__

#endif // __GNUC__
};

std::string MonetDBCtn::name() const {
  static auto name_str = "MonetDB";
  return name_str;
}
