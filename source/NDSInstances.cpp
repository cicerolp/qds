#include "NDSInstances.h"

void NDSInstances::run(const std::vector<Schema> &args) {
  for (const auto &schema : args) {
    NDSInstances::getInstance()._container.emplace(schema.name, std::make_shared<NDS>(schema));
  }
}

std::string NDSInstances::query(const Query &query) {
  auto cube = get_instance(query.get_dataset());

  if (!cube) {
    return ("[]");
  } else {
    return cube->query(query);
  }
}

std::string NDSInstances::schema(const std::string &instance) const {
  auto cube = get_instance(instance);

  if (!cube) return ("[]");

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();
  // TODO return schema from cube
  writer.EndObject();

  return buffer.GetString();
}
