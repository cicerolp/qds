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

    bool debug{true};

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

    Server::server_opts nds_opts;
    nds_opts.port = 7000;

#ifdef ENABLE_TIMMING
    nds_opts.cache = false;
    nds_opts.multithreading = false;
#else
    nds_opts.cache = true;
    nds_opts.multithreading = true;
#endif // ENABLE_TIMMING

    using namespace popl;

    // declare the supported options
    OptionParser op("\nCommand Line Arguments");

    auto help_option = op.add<Switch>("h", "help", "produce help message");
    auto debug_option = op.add<Switch>("n", "no-log", "disable debug info");

    op.add<Implicit<uint32_t>>("", "pid", "send signal to pid", nds_opts.pid, &nds_opts.pid);
    op.add<Value<uint32_t>>("d", "depth", "quadtree depth", nds_opts.quadtree_depth, &nds_opts.quadtree_depth);

    // http server
    auto server_option = op.add<Switch>("s", "no-server", "disable http server");
    auto cache_option = op.add<Switch>("c", "no-cache", "disable http cache");
    op.add<Value<uint32_t>>("p", "port", "http server port", nds_opts.port, &nds_opts.port);

    auto xml_files = op.add<Value<std::string>>("x", "xml", "input files");

    op.parse(argc, argv);

    if (help_option->is_set()) {
      std::cout << op << std::endl;
      exit(0);
    }

    if (debug_option->is_set())
      nds_opts.debug = debug_option->value();

    if (server_option->is_set())
      nds_opts.server = server_option->value();

    if (cache_option->is_set())
      nds_opts.cache = cache_option->value();

    if (xml_files->is_set()) {
      for (size_t n = 0; n < xml_files->count(); ++n) {
        auto str = xml_files->value(n);

        nds_opts.input_files.emplace_back(str);
        nds_opts.schemas.emplace_back(str);
      }
    }

    g_Quadtree_Depth = nds_opts.quadtree_depth;

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