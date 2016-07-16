#include "stdafx.h"
#include "NDSInstances.h"

void NDSInstances::run(const std::vector<Schema>& args) {
	for (const auto& schema : args) {
		NDSInstances::getInstance()._container.emplace(schema.name, std::make_shared<NDS>(schema));
	}
}

std::string NDSInstances::query(const Query& query) {

   auto it = _container.find(query.instance());

   if (it == _container.end()) {
      return ("");
   } else {
      return (*it).second->query(query);
   }
}
