#include "stdafx.h"
#include "NDSInstances.h"

void NDSInstances::run(const std::vector<Schema>& args) {
	for (const auto& schema : args) {
		NDSInstances::getInstance()._container.emplace(schema.name, std::make_shared<NDS>(schema));
	}
}
