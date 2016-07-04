#include "stdafx.h"
#include "Schema.h"

#include "NDSInstances.h"


int main(int argc, char *argv[]) {

   std::vector<Schema> schemas;
   std::vector<std::string> inputFiles;

   try {
      if (argc < 2) {
         inputFiles.emplace_back("./xml/example.xml");
      } else {
         for (int i = 1; i < argc; i++) {
            if (argv[i][0] != '-') {
               inputFiles.emplace_back(argv[i]);
            } else { }
         }
      }
   } catch (...) {
      std::cout << "error: invalid arguments" << std::endl;
      exit(0);
   }

   std::cout << "XML Files:" << std::endl;

   for (const auto& str : inputFiles) {
      std::cout << "\t" + str << std::endl;
      schemas.emplace_back(str);
   }

   //std::thread t1(HashedCubeInstances::write, args);
   //t1.join();   

   NDSInstances& nds = NDSInstances::getInstance();

   std::cout << getCurrentRSS() / (1024 * 1024) << std::endl;

   return 0;
}

