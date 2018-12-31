#pragma once
#include "GeoCtnIntf.h"

class PostGisCtn : public GeoCtnIntf {
 public:
  PostGisCtn(int argc, char *argv[]);

  ~PostGisCtn();

  // snap
  void create_snap() override;
  void insert_snap(const std::string &filename) override;

  // on-time
  void create_on_time() override;
  void insert_on_time(const std::string &filename) override;

  // small-twitter
  void create_small_twitter() override;
  void insert_small_twitter(const std::string &filename) override;

  // gaussian
  void create_gaussian() override;
  void insert_gaussian(const std::string &filename) override;

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
