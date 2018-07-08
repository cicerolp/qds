#include "stdafx.h"
#include "NDSInstances.h"
#include "Server.h"

int main(int argc, char *argv[]) {
  /*
  TIMER_DECLARE
  TIMER_START

  PRINTCSVF("FUN");

  TIMER_END
  TIMER_OUTPUT("FUN")

  exit(0);
  */

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

  std::cout << nds_opts << std::endl;

  std::unique_ptr<std::thread> server_ptr;

  // http server
  if (nds_opts.server) {
    server_ptr = std::make_unique<std::thread>(Server::run, nds_opts);
  }

  // nds instances
  std::thread instances_run(NDSInstances::run, nds_opts.schemas);

  instances_run.join();
  std::cout << "\nCurrent Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;

  if (server_ptr) {
    std::cout << "Server Running... Press any key to exit." << std::endl;
    getchar();

    Server::getInstance().stop();
    server_ptr->join();
  }

  return 0;
}
