#include "stdafx.h"
#include "NDSInstances.h"
#include "Server.h"

#include <signal.h>

int main(int argc, char *argv[]) {
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

  auto nds_opts = Server::init(argc, argv);

  if (nds_opts.debug_info) {
    std::cout << nds_opts << std::endl;
  }

  std::unique_ptr<std::thread> server_ptr;

  // http server
  if (nds_opts.server) {
    server_ptr = std::make_unique<std::thread>(Server::run, nds_opts);
  }

  // nds instances
  std::thread instances_run(NDSInstances::run, nds_opts);
  instances_run.join();

  if (nds_opts.debug_info) {
    std::cout << "\nCurrent Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;
  }

  if (server_ptr) {
    if (nds_opts.debug_info) {
      std::cout << "Server Running... Press any key to exit." << std::endl;
    }

    if (nds_opts.pid != 0) {
      std::cout << "sending signal to: " << nds_opts.pid << std::endl;
      kill(nds_opts.pid, SIGUSR1);
    }

    getchar();

    Server::getInstance().stop();
    server_ptr->join();

  } else {
    if (nds_opts.pid != 0) {
      std::cout << "sending signal to: " << nds_opts.pid << std::endl;
      kill(nds_opts.pid, SIGUSR1);
    }
  }

  return 0;
}
