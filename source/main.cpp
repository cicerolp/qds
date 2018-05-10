#include "NDSInstances.h"
#include "Server.h"

uint32_t g_Quadtree_Depth{18};

int main(int argc, char *argv[]) {
  /*static const float radius = 10.f;

  for (auto theta = 0.0; theta <= 2.0 * M_PI; theta += (2.0 * M_PI) / 20.0) {

    // pdigest c1
    auto theta_c1 = 0;
    auto x1 = radius * std::cos(theta_c1);
    auto y1 = radius * std::sin(theta_c1);

    // pdigest c2
    auto theta_c2 = theta;
    auto x2 = radius * std::cos(theta_c2);
    auto y2 = radius * std::sin(theta_c2);


    auto d = std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

    std::cout << d << std::endl;

  }

  exit(0);*/


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // categorical //
  // [dimension_name].values.([value_0]:[value_1]:...:[value_N])

  // temporal //
  // [dimension_name].interval.([lower_bound]:[upper_bound])
  // [dimension_name].sequence.([lower_bound]:[interval_width]:[num_intervals]:[stride])

  // spatial //
  // [dimension_name].tile.([x]:[y]:[z]:[resolution])
  // [dimension_name].region.([x0]:[y0]:[x1]:[y1]:[z])
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

  std::vector<std::string> input_files;

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
      "cache,c",
      po::value<bool>(&nds_opts.cache)->default_value(nds_opts.cache),
      "web cache");

  desc.add_options()(
      "depth,d",
      po::value<uint32_t>(&g_Quadtree_Depth)->default_value(g_Quadtree_Depth),
      "quadtree depth");

  desc.add_options()("xml,x",
                     po::value<std::vector<std::string>>(&input_files)
                         ->default_value(std::vector<std::string>(
                             1, "./xml/brightkite.nds.xml"),
                                         "./xml/brightkite.nds.xml")
                         ->composing(),
                     "input files");

  po::positional_options_description p;
  p.add("xml", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    exit(0);
  }

  if (vm.count("no-server")) {
    server = false;
  }

  std::cout << "Server Options:" << std::endl;
  std::cout << "\tOn/Off: " << server << std::endl;
  std::cout << "\t" << nds_opts << std::endl;

  std::cout << "NDS Options:" << std::endl;
  std::cout << "\tQuadtree Depth: " << g_Quadtree_Depth << std::endl;
  std::cout << std::endl;

  std::vector<Schema> schemas;

  std::cout << "XML Files:" << std::endl;
  for (const auto &str : input_files) {
    std::cout << "\t" + str << std::endl;
    schemas.emplace_back(str);
  }
  std::cout << std::endl;

  std::unique_ptr<std::thread> server_ptr;

  // http server
  if (server) {
    server_ptr = std::make_unique<std::thread>(Server::run, nds_opts);
  }

  // nds instances
  std::thread instances_run(NDSInstances::run, schemas);

  instances_run.join();
  std::cout << "Current Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;

  if (server_ptr) {
    std::cout << "Server Running... Press any key to exit." << std::endl;
    getchar();

    Server::getInstance().stop();
    server_ptr->join();
  }

  return 0;
}
