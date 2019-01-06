//
// Created by cicerolp on 11/26/18.
//

#include "stdafx.h"
#include "Query.h"
#include "Server.h"

#include "MonetDBCtn.h"
#include "PostGisCtn.h"
#include "SpatiaLiteCtn.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

template<typename T>
void run_bench(int argc, char *argv[], const std::string &log, const std::string &data, const std::string &schema) {
  auto container = std::make_unique<T>(argc, argv);

  if (schema == "snap") {
    container->create_snap();
    container->insert_snap(data);
  } else if (schema == "on-time") {
    container->create_on_time();
    container->insert_on_time(data);
  } else if (schema == "twitter") {
    container->create_small_twitter();
    container->insert_small_twitter(data);
  } else if (schema == "gaussian") {
    container->create_gaussian();
    container->insert_gaussian(data);
  } else {
    // invalid schema
    return;
  }

  // log file
  std::ifstream infile(log);

  std::string line;
  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    auto volatile field = container->query(Query(line));

    TIMER_RESET_IT
    TIMER_INCR_ID
  }

  infile.close();
}

int main(int argc, char *argv[]) {

  std::string schema = "snap";
  std::string data = "input.csv";
  std::string log = "input.log";

  // declare the supported options
  po::options_description desc("Command Line Arguments");

  desc.add_options()("input,i", po::value<std::string>(&log)->default_value(log), "log file");
  desc.add_options()("data,d", po::value<std::string>(&data)->default_value(data), "data file");
  desc.add_options()("schema,s", po::value<std::string>(&schema)->default_value(schema), "schema");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  ////////////////////////////////////////////////////////
  
  run_bench<SpatiaLiteCtn>(argc, argv, log, data, schema);
  run_bench<MonetDBCtn>(argc, argv, log, data, schema);
  run_bench<PostGisCtn>(argc, argv, log, data, schema);

  return 0;
}
