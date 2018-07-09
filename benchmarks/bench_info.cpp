#include "stdafx.h"
#include "Query.h"
#include "Server.h"
#include "NDSInstances.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

void run_bench(const Server::server_opts &opts) {
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();

  std::thread instances_run(NDSInstances::run, opts);
  instances_run.join();

  end = std::chrono::high_resolution_clock::now();
  long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();

  writer.Key("memory_usage");
  writer.Uint64(getCurrentRSS() / (1024 * 1024));

  writer.Key("time");
  writer.Uint64(duration);

#ifdef ENABLE_METRICS
  writer.Key("pivots");
  writer.Uint64(PIVOTS_N);

  writer.Key("shared_pivots");
  writer.Uint64(SHARED_PIVOTS_N);
#endif // ENABLE_METRICS


  writer.EndObject();

  std::cout << buffer.GetString() << std::endl;
}

int main(int argc, char *argv[]) {
  auto nds_opts = Server::init(argc, argv);

  run_bench(nds_opts);

  return 0;
}