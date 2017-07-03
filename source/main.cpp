#include "NDSInstances.h"
#include "Schema.h"
#include "Server.h"
#include "stdafx.h"

uint32_t g_Quadtree_Depth{25};

int main(int argc, char* argv[]) {
  if (std::getenv("NDS_DATA") == nullptr) {
    std::cerr << "error: invalid environment path %NDS_DATA%" << std::endl;
    exit(-1);
  } else {
    std::cout << "NDS_DATA: <" << std::getenv("NDS_DATA") << ">" << std::endl;
  }

  namespace po = boost::program_options;

  bool server = true;
  Server::server_opts nds_opts;
  nds_opts.port = 7000;
  nds_opts.cache = true;
  nds_opts.multithreading = true;

  bool telemetry = false;
  bool benchmark = false;

  uint32_t benchmark_passes = 10;

  std::vector<std::string> input_files;
  std::vector<std::string> benchmark_files;

  // Declare the supported options.
  po::options_description desc("\nCommand Line Arguments");
  desc.add_options()("help,h", "produce help message");

  desc.add_options()("no-server,s", "disable server")("telemetry,t",
                                                      "enable telemetry");

  desc.add_options()(
      "port,p",
      po::value<uint32_t>(&nds_opts.port)->default_value(nds_opts.port),
      "server port");

  desc.add_options()(
      "depth,d",
      po::value<uint32_t>(&g_Quadtree_Depth)->default_value(g_Quadtree_Depth),
      "quadtree depth");

  desc.add_options()(
      "passes,p",
      po::value<uint32_t>(&benchmark_passes)->default_value(benchmark_passes),
      "number of evaluatinons");

  desc.add_options()("xml,x",
                     po::value<std::vector<std::string>>(&input_files)
                         ->default_value(std::vector<std::string>(
                                             1, "./xml/brightkite.nds.xml"),
                                         "./xml/brightkite.nds.xml")
                         ->composing(),
                     "input files");

  desc.add_options()("log,l",
                     po::value<std::vector<std::string>>(&benchmark_files),
                     "benchmark input files");

  po::positional_options_description p;
  p.add("xml", -1);

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    exit(0);
  }

  if (vm.count("no-server")) {
    server = false;
  }

  if (vm.count("telemetry")) {
    telemetry = false;
  }

  if (vm.count("log")) {
    benchmark = true;
  }

  // disable server if benchmarking
  if (benchmark) server = false;

  // disable server multithreading
  if (telemetry) nds_opts.multithreading = false;

  std::cout << "Server Options:" << std::endl;
  std::cout << "\tOn/Off: " << server << std::endl;
  std::cout << "\t" << nds_opts << std::endl;

  std::cout << "NDS Options:" << std::endl;
  std::cout << "\tQuadtree Depth: " << g_Quadtree_Depth << std::endl;
  std::cout << std::endl;

  std::vector<Schema> schemas;

  std::cout << "XML Files:" << std::endl;
  for (const auto& str : input_files) {
    std::cout << "\t" + str << std::endl;
    schemas.emplace_back(str);
  }
  std::cout << std::endl;

  if (benchmark) {
    std::cout << "Benchmark Options:" << std::endl;
    std::cout << "\tNumber of Passes: " << benchmark_passes << std::endl;
    std::cout << std::endl;

    std::cout << "Benchmark Files:" << std::endl;
    for (const auto& str : benchmark_files) {
      std::cout << "\t" + str << std::endl;
    }
  }

  std::unique_ptr<std::thread> server_ptr;

  // http server
  if (server) {
    server_ptr = std::make_unique<std::thread>(Server::run, nds_opts);
  }

  // nds instances
  std::thread instances_run(NDSInstances::run, schemas, telemetry);

  instances_run.join();
  std::cout << "Current Resident Size: " << getCurrentRSS() / (1024 * 1024)
            << " MB" << std::endl;

  if (benchmark) {
    std::vector<Query> queries;

    std::cout << "Reading benchmark files... ";
    for (auto& file : benchmark_files) {
      std::ifstream infile(file);

      while (!infile.eof()) {
        std::string line;
        std::getline(infile, line);

        if (!line.empty()) queries.emplace_back(line);
      }

      infile.close();
    }
    std::cout << "Done." << std::endl;

    for (uint32_t pass = 0; pass < benchmark_passes; ++pass) {
      std::cout << "[" + std::to_string(pass + 1) + " of " +
                       std::to_string(benchmark_passes) + "] Running " +
                       std::to_string(queries.size()) + " queries... ";

      for (auto& query : queries) {
        NDSInstances::getInstance().query(query);
      }
      std::cout << "Done." << std::endl;
    }
  }

  if (server_ptr) {
    std::cout << "Server Running... Press any key to exit." << std::endl;
    getchar();

    Server::getInstance().stop();
    server_ptr->join();
  }

  return 0;
}
