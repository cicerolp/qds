#pragma once

#include "Query.h"
#include "Schema.h"
#include "Singleton.h"

extern uint32_t g_Quadtree_Depth;

class Server : public Singleton<Server> {
  friend class Singleton<Server>;

 public:
  struct server_opts {
    uint32_t quadtree_depth{18};

    bool server{true};
    uint32_t port{8000};

    uint32_t pid{0};

    bool cache{true};
    bool multithreading{true};

    bool debug_info{true};

    std::vector<Schema> schemas;
    std::vector<std::string> input_files;

    friend std::ostream &operator<<(std::ostream &os, const server_opts &rhs) {
      os << "NDS_DATA: <" << std::getenv("NDS_DATA") << ">" << std::endl;

      os << "Server Options:" << std::endl;
      os << "\tOn/Off: " << rhs.server << std::endl;
      os << "\t" << "port: " << rhs.port << ", cache: " << rhs.cache
         << ", multithreading: " << rhs.multithreading
         << ", pid: " << rhs.pid << std::endl;

      std::cout << "\nNDS Options:" << std::endl;
      std::cout << "\tQuadtree Depth: " << rhs.quadtree_depth << std::endl;
      std::cout << std::endl;

      std::cout << "XML Files:" << std::endl;
      for (const auto &str : rhs.input_files) {
        std::cout << "\t" + str << std::endl;
      }

      return os;
    }
  };

  static void run(server_opts opts);
  static void handler(struct mg_connection *conn, int ev, void *p);

  static void printText(struct mg_connection *conn, const std::string &content);
  static void printJson(struct mg_connection *conn, const std::string &content);

  static server_opts init(int argc, char *argv[]) {
    if (std::getenv("NDS_DATA") == nullptr) {
      std::cerr << "error: invalid environment path %NDS_DATA%" << std::endl;
      exit(-1);
    }

    namespace po = boost::program_options;

    Server::server_opts nds_opts;
    nds_opts.port = 7000;

#ifdef ENABLE_TIMMING
    nds_opts.cache = false;
    nds_opts.multithreading = false;
#else
    nds_opts.cache = true;
    nds_opts.multithreading = true;
#endif // ENABLE_TIMMING

    // Declare the supported options.
    po::options_description desc("\nCommand Line Arguments");

    desc.add_options()("help,h", "produce help message");
    desc.add_options()("no-log,n", "produce help message");

    desc.add_options()("no-server,s", "disable server")(
        "telemetry,t", "enable telemetry");

    desc.add_options()(
        "pid",
        po::value<uint32_t>(&nds_opts.pid)->default_value(nds_opts.pid),
        "send signal to pid");

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
        po::value<uint32_t>(&nds_opts.quadtree_depth)->default_value(nds_opts.quadtree_depth),
        "quadtree depth");

    desc.add_options()(
        "xml,x",
        po::value<std::vector<std::string>>(&nds_opts.input_files)
            ->default_value(std::vector<std::string>(1, "file.xml"), "file.xml")
            ->multitoken()->composing(),
        "input files");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      exit(0);
    }

    if (vm.count("no-log")) {
      nds_opts.debug_info = false;
    }

    if (vm.count("no-server")) {
      nds_opts.server = false;
    }

    g_Quadtree_Depth = nds_opts.quadtree_depth;

    for (const auto &str : nds_opts.input_files) {
      nds_opts.schemas.emplace_back(str);
    }
    std::cout << std::endl;

    return nds_opts;
  }

  void stop() { running = false; };

 private:
  bool running{true};

  struct mg_serve_http_opts http_server_opts;
  struct mg_connection *nc;
  struct mg_mgr mgr;

  server_opts nds_opts;

  Server() = default;
  virtual ~Server() = default;
};