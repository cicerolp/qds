#include "stdafx.h"
#include "NDS.h"

NDS::NDS(const Schema& schema) {
   
   _data = std::make_unique<Data>(schema.file);

   std::cout << "\nBuildind NDS: " << std::endl;
   std::cout << "\tName: " << schema.name << std::endl;
   std::cout << "\tSize: " << _data->size() << std::endl;

   building_container current, expand;

   //TODO
   //current = cube->root();
   current.emplace_back(0, _data->size());

   // categorical
   for (const auto& tuple : schema.categorical) {
	   _categorical[std::get<0>(tuple)] = std::make_unique<CategoricalDimension>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
	   _categorical[std::get<0>(tuple)]->build(current, expand, (*_data));
   }
   
   // TODO
   //data->dispose();
}

NDS::~NDS() {

}

std::string NDS::query(const Query& query) {
   return " ";
}
