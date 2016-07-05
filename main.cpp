#include "stdafx.h"
#include "Schema.h"

#include "NDSInstances.h"


int main(int argc, char *argv[]) {

   std::vector<Schema> schemas;
   std::vector<std::string> inputFiles;

   try {
      if (argc < 2) {
         inputFiles.emplace_back("./xml/delay.xml");
      } else {
         for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
               inputFiles.emplace_back(argv[i]);
            } else { }
         }
      }
   } catch (...) {
      std::cerr << "error: invalid arguments" << std::endl;
      exit(0);
   }

   std::cout << "XML Files:" << std::endl;

   for (const auto& str : inputFiles) {
      std::cout << "\t" + str << std::endl;
      schemas.emplace_back(str);
   }

   NDSInstances& instances = NDSInstances::getInstance();

   std::thread instances_run(NDSInstances::run, schemas);
   instances_run.join();

   std::cout << "Current Resident Size: " << getCurrentRSS() / (1024 * 1024) << " MB" << std::endl;

   return 0;
}

