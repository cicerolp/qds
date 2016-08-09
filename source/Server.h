#pragma once

#include "Singleton.h"
#include "NDSInstances.h"

#include "Query.h"

class Server: public Singleton<Server> {
	friend class Singleton<Server>;

public:
   struct server_opts {
      uint32_t port{ 8000 };

      bool cache{ true };
      bool multithreading{ true };

      friend std::ostream& operator<<(std::ostream& os, const server_opts& obj) {
         return os
            << "port: " << obj.port
            << ", cache: " << obj.cache
            << ", multithreading: " << obj.multithreading << std::endl;
      }
   };

	static void run(server_opts opts);
	static void handler(struct mg_connection* conn, int ev, void *p);

	static void printText(struct mg_connection* conn, const std::string& content);
	static void printJson(struct mg_connection* conn, const std::string& content);

	void stop() { running = false; };

private:
	bool running{ true };

	struct mg_serve_http_opts http_server_opts;
	struct mg_connection *nc;
	struct mg_mgr mgr;

   server_opts nds_opts;

	Server() = default;
	virtual ~Server() = default;
};