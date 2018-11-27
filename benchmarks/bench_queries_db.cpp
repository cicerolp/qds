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
void run_bench(int argc, char *argv[], const std::string &log, const std::string &data) {
  auto container = std::make_unique<T>(argc, argv);

  // create database
  container->create();

  // insert into table
  container->insert(data);

  // log file
  std::ifstream infile(log);

  std::string line;
  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    container->query(Query(line));

    TIMER_INCR_IT

    container->query(Query(line));

    TIMER_INCR_IT

    container->query(Query(line));

    TIMER_RESET_IT
    TIMER_INCR_ID
  }

  infile.close();
}

int main(int argc, char *argv[]) {

  std::string data = "input.csv";
  std::string log = "input.log";

  // declare the supported options
  po::options_description desc("Command Line Arguments");

  desc.add_options()("input,i", po::value<std::string>(&log)->default_value(log), "log file");
  desc.add_options()("data,d", po::value<std::string>(&data)->default_value(data), "data file");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  ////////////////////////////////////////////////////////

  // run_bench<PostGisCtn>(argc, argv, log, data);
  // run_bench<SpatiaLiteCtn>(argc, argv, log, data);
  run_bench<MonetDBCtn>(argc, argv, log, data);

  return 0;
}