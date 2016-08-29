#include "stdafx.h"
#include "Schema.h"
#include "Server.h"
#include "NDSInstances.h"
#include "SpatialElement.h"

int main(int argc, char* argv[]) {
   if (std::getenv("NDS_DATA") == nullptr) {
      std::cerr << "error: invalid environment path %NDS_DATA%" << std::endl;
      exit(-1);
   } else {
      std::cout << "NDS_DATA: <"<< std::getenv("NDS_DATA") << ">" << std::endl;
   }

   bool server = true;
   Server::server_opts nds_opts;
   nds_opts.port = 7000;
   nds_opts.cache = false;
   nds_opts.multithreading = true;

   bool telemetry = false;

   bool benchmark = false;
   uint32_t benchmark_passes = 10;
   std::vector<std::string> benchmark_files;

   /*
   telemetry = true;
   benchmark = true;

   benchmark_files.emplace_back("./csv/flights-bench.csv");

   //benchmark_files.emplace_back("./csv/brightkite.csv");
   //benchmark_files.emplace_back("./csv/brightkite-bench.csv");
   /**/

   std::vector<Schema> schemas;
   std::vector<std::string> inputFiles;

   try {
      if (argc < 2) {
         //inputFiles.emplace_back("./xml/brightkite-example.nds.xml");

         inputFiles.emplace_back("./xml/brightkite.nds.xml");
         //inputFiles.emplace_back("./xml/brightkite.nds-leaf.xml");
         
         //inputFiles.emplace_back("./xml/gowalla.nds.xml");
         //inputFiles.emplace_back("./xml/gowalla.nds-leaf.xml");

         //inputFiles.emplace_back("./xml/performance-example.nds.xml");
         //inputFiles.emplace_back("./xml/performance.nds.xml");
         //inputFiles.emplace_back("./xml/performance.nds-leaf.xml");

         //inputFiles.emplace_back("./xml/twitter-small.nds.xml");
         //inputFiles.emplace_back("./xml/twitter-small.nds-leaf.xml");
      } else {
         for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
               inputFiles.emplace_back(argv[i]);
            } else {
               std::string arg = argv[i];
               if (arg == std::string("-telemetry")) {
                  telemetry = true;
               } else if (arg == "-no-server") {
                  server = false;
               } else if (arg == "-log") {
                  benchmark = true;
                  benchmark_files.emplace_back(argv[++i]);
               } else if (arg == "-port") {
                  try {
                     nds_opts.port = std::stoul(argv[++i]);
                  } catch (...) {
                     std::cerr << "error: invalid server port, using " << nds_opts.port << std::endl;
                  }
               }
            }
         }
      }
   } catch (...) {
      std::cerr << "error: invalid arguments" << std::endl;
      exit(-1);
   }

   // disable server
   if (benchmark) server = false;

   // disable server multithreading
   if (telemetry) nds_opts.multithreading = false;

   std::cout << "Server Options:" << std::endl;
   std::cout << "\tOn/Off: " << server << std::endl;
   std::cout << "\t" << nds_opts << std::endl;

   std::cout << "XML Files:" << std::endl;
   for (const auto& str : inputFiles) {
      std::cout << "\t" + str << std::endl;
      schemas.emplace_back(str);
   }

   if (benchmark) {
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
   std::cout << "Current Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;

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
         /*std::cout << "["
            + std::to_string(pass + 1)
            + " of "
            + std::to_string(benchmark_passes)
            + "] Running "
            + std::to_string(queries.size())
            + " queries... ";*/

         for (auto& query : queries) {
            NDSInstances::getInstance().query(query);
         }
         /*std::cout << "Done." << std::endl;*/
      }
   }

   if (server_ptr) {
      std::cout << "Server Running..." << std::endl;
      getchar();

      Server::getInstance().stop();
      server_ptr->join();
   }

   return 0;
}

