#pragma once
#include "GeoCtnIntf.h"

class PostGisCtn : public GeoCtnIntf {
 public:
  PostGisCtn(int argc, char *argv[]);

  ~PostGisCtn();

  // build container
  void create() override;

  // update container
  void insert(const std::string &filename) override;

  void query(const Query &query) override;

  inline virtual std::string name() const override;

 private:
  bool _init{false};
#ifdef __GNUC__
  PGconn *_conn;
#endif // __GNUC__
};

std::string PostGisCtn::name() const {
  static auto name_str = "PostgreSQL";
  return name_str;
}
