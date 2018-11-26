#pragma once
#include "GeoCtnIntf.h"

class SpatiaLiteCtn : public GeoCtnIntf {
 public:
  SpatiaLiteCtn(int argc, char* argv[]);

  ~SpatiaLiteCtn();

  // build container
  void create() override;

  // update container
  void insert(const std::string &filename) override;

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
