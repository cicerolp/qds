#pragma once

#include "Singleton.h"
#include "NDSInstances.h"

#include "Query.h"

class Server: public Singleton<Server> {
	friend class Singleton<Server>;

public:
	static void run(bool disable_multithreading, uint32_t port);
	static void handler(struct mg_connection* conn, int ev, void *p);

	static void printText(struct mg_connection* conn, const std::string& content, int code);
	static void printJson(struct mg_connection* conn, const std::string& content, int code);

	void stop() { running = false; };

private:
	bool running{ true };

	struct mg_serve_http_opts http_server_opts;
	struct mg_connection *nc;
	struct mg_mgr mgr;

	Server() = default;
	virtual ~Server() = default;
};