#pragma once
#include "GeoCtnIntf.h"

class SpatiaLiteCtn : public GeoCtnIntf {
 public:
  SpatiaLiteCtn(int argc, char* argv[]);

  ~SpatiaLiteCtn();

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
  static void notice(const char* fmt, ...);

  static void log_and_exit(const char* fmt, ...);

 private:
  bool _init{false};
  void* _cache{nullptr};
#ifdef __GNUC__
  sqlite3* _handle{nullptr};
#endif // __GNUC__
};

std::string SpatiaLiteCtn::name() const {
  static auto name_str = "SQLite";
  return name_str;
}
