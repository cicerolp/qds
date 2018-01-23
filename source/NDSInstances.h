#pragma once

#include "NDS.h"
#include "Schema.h"
#include "Singleton.h"

class NDSInstances : public Singleton<NDSInstances> {
  friend class Singleton<NDSInstances>;

 public:
  static void run(const std::vector<Schema> &args);

  std::string query(const Query &query);
  std::string schema(const std::string &instance) const;

 private:
  inline std::shared_ptr<NDS> get_instance(const std::string &instance) const {
    auto it = _container.find(instance);

    if (it == _container.end()) {
      return nullptr;
    } else {
      return (*it).second;
    }
  };

  NDSInstances() = default;
  virtual ~NDSInstances() = default;

  bool _ready{false};
  std::unordered_map<std::string, std::shared_ptr<NDS>> _container;
};
