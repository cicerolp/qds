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

std::string NDSInstances::pipeline(const Pipeline &pipeline) {
  auto cube = get_instance(pipeline.get_dataset());

  if (!cube) {
    return ("[]");
  } else {
    return cube->pipeline(pipeline);
  }
}

std::string NDSInstances::augmented_series(const AugmentedSeries &augmented_series) {
  auto cube = get_instance(augmented_series.get_pipeline().get_dataset());

  if (!cube) {
    return ("[]");
  } else {
    return cube->augmented_series(augmented_series);
  }
}

std::string NDSInstances::schema(const std::string &url) const {
  boost::char_separator<char> sep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(url, sep);

  for (auto &it : tokens) {
    std::vector<std::string> clausules;
    boost::split(clausules, it, boost::is_any_of("="));

    if (clausules.size() == 2) {

      auto &key = clausules[0];
      auto &value = clausules[1];

      if (key == "dataset") {

        auto cube = get_instance(value);

        if (!cube) return ("[]");

        return cube->schema();
      }
    }
  }

  return "[]";
}
