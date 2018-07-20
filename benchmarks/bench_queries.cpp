#include "stdafx.h"
#include "Query.h"
#include "Server.h"
#include "NDSInstances.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

void run_bench(const std::string &input) {
  std::cout << "\nLoading log file: <" << input << ">" << std::endl;

  std::ifstream infile(input);

  std::string line;
  while (!infile.eof()) {

    std::getline(infile, line);

    if (line.empty()) {
      continue;
    }

    for (auto i = 0; i < 3; ++i) {
      NDSInstances::getInstance().query(Query(line));

      TIMER_INCR_IT
    }

    TIMER_RESET_IT
    TIMER_INCR_ID
  }

  infile.close();
}

int main(int argc, char *argv[]) {
  auto nds_opts = Server::init(argc, argv);

  std::string input = "input.log";

  // declare the supported options
  po::options_description desc("Command Line Arguments");

  desc.add_options()("input,i", po::value<std::string>(&input)->default_value(input),
                     "input file");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  ////////////////////////////////////////////////////////

  // nds instances
  std::thread instances_run(NDSInstances::run, nds_opts);
  instances_run.join();

  run_bench(input);

  return 0;
}
