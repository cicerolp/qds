#pragma once

#include "Singleton.h"
#include "Schema.h"
#include "NDS.h"

class NDSInstances : public Singleton<NDSInstances> {
	friend class Singleton<NDSInstances>;
public:
	static void run(const std::vector<Schema>& args);

	std::string query(const Query& query);

private:
	NDSInstances() = default;
	virtual ~NDSInstances() = default;

	bool _ready{ false };
	std::unordered_map<std::string, std::shared_ptr<NDS>> _container;
};
