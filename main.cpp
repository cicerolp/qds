#include "stdafx.h"
#include "Schema.h"
#include "Server.h"
#include "NDSInstances.h"


int main(int argc, char *argv[]) {

   
   bool no_server = false;

   Server::server_opts nds_opts;
   nds_opts.port = 8001;
   nds_opts.cache = true;
   nds_opts.multithreading = true;   

   if (std::getenv("NDS_DATA") == nullptr) {
      std::cerr << "error: invalid environment path %NDS_DATA%" << std::endl;
      exit(-1);
   }

   std::vector<Schema> schemas;
   std::vector<std::string> inputFiles;

   try {
      if (argc < 2) {         
         //inputFiles.emplace_back("./xml/brightkite-example.nds.xml");

         inputFiles.emplace_back("./xml/brightkite.nds.xml");
         //inputFiles.emplace_back("./xml/gowalla.nds.xml");
         
         //inputFiles.emplace_back("./xml/delay.nds.xml");
         //inputFiles.emplace_back("./xml/performance.nds.xml");

         //inputFiles.emplace_back("./xml/twitter-small.nds.xml");
      } else {
         for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
               inputFiles.emplace_back(argv[i]);
            } else { }
         }
      }
   } catch (...) {
      std::cerr << "error: invalid arguments" << std::endl;
      exit(-1);
   }

   std::cout << "XML Files:" << std::endl;

   for (const auto& str : inputFiles) {
      std::cout << "\t" + str << std::endl;
      schemas.emplace_back(str);
   }

   std::unique_ptr<std::thread> server;

   // http server
   if (!no_server) {      
      server = std::make_unique<std::thread>(Server::run, nds_opts);
   }
   
   // nds instances
   std::thread instances_run(NDSInstances::run, schemas);
   
   instances_run.join();
   std::cout << "Current Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;

   /*Query query("/rest/query/brightkite/tile/tile/0/0/0/0/8");
   for (int i = 0; i < 100; ++i) {
      NDSInstances::getInstance().query(query);
   }*/

   if (server) {
      std::cout << "Server Running..." << std::endl;
      getchar();

      Server::getInstance().stop();
      server->join();
   }

   return 0;
}

