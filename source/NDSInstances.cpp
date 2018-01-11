#include "NDSInstances.h"

void NDSInstances::run(const std::vector<Schema>& args, bool telemetry) {
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

  NDSInstances::getInstance()._telemetry = telemetry;
  for (const auto& schema : args) {
    start = std::chrono::high_resolution_clock::now();
    NDSInstances::getInstance()._container.emplace(
        schema.name, std::make_shared<NDS>(schema));
    end = std::chrono::high_resolution_clock::now();

    if (telemetry) {
      auto time = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());
      std::stringstream stream;
      stream << std::put_time(std::localtime(&time), "-%Y-%m-%d-%H-%M-%S.csv");

      NDSInstances::getInstance()._telemetry_files.emplace(
          schema.name, std::make_unique<std::ofstream>(
                           schema.name + stream.str(), std::ofstream::out));

      (*NDSInstances::getInstance()._telemetry_files[schema.name])
          << "sep=," << std::endl;

      (*NDSInstances::getInstance()._telemetry_files[schema.name])
          << "memory,time2build" << std::endl;

      auto duration =
          std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
      auto memory = getCurrentRSS() / (1024 * 1024);

      (*NDSInstances::getInstance()._telemetry_files[schema.name])
          << memory << "," << duration << std::endl;
    }
  }
}

std::string NDSInstances::query(const Query& query) {
  auto cube = get_instance(query.get_instance());

  if (!cube) {
    return ("[]");
  } else {
    return cube->query(query, _telemetry_files[query.get_instance()].get());
  }
}

std::string NDSInstances::schema(const std::string& instance) const {
  auto cube = get_instance(instance);

  if (!cube) return ("[]");

  interval_t interval = cube->get_interval();

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();
  writer.String("totalcount");
  writer.Int(cube->size());
  writer.String("mindate");
  writer.Uint(interval.bound[0]);
  writer.String("maxdate");
  writer.Uint(interval.bound[1]);
  writer.EndObject();

  return buffer.GetString();
}
