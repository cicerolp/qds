#pragma once

#include "Singleton.h"
#include "Schema.h"
#include "NDS.h"

class NDSInstances : public Singleton<NDSInstances> {
   friend class Singleton<NDSInstances>;
public:
   static void run(const std::vector<Schema>& args);

private:
   NDSInstances();
   virtual ~NDSInstances();

   bool _ready{ false };
   std::unordered_map<std::string, std::shared_ptr<NDS>> _container;
};
