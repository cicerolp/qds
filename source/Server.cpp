#include "stdafx.h"
#include "Server.h"

void Server::run(server_opts opts) {
   Server::getInstance().nds_opts = opts;

   mg_mgr_init(&Server::getInstance().mgr, nullptr);
   Server::getInstance().nc =
      mg_bind(&Server::getInstance().mgr, std::to_string(Server::getInstance().nds_opts.port).c_str(), handler);

   mg_set_protocol_http_websocket(Server::getInstance().nc);

   Server::getInstance().http_server_opts.document_root = "WebContent";

   if (Server::getInstance().nds_opts.multithreading) {
      mg_enable_multithreading(Server::getInstance().nc);
   }

   while (Server::getInstance().running) {
      mg_mgr_poll(&Server::getInstance().mgr, 1);
   }
   mg_mgr_free(&Server::getInstance().mgr);
}

void Server::handler(mg_connection* conn, int ev, void* p) {
   if (ev != MG_EV_HTTP_REQUEST) return;

   struct http_message* hm = (struct http_message *) p;
   std::string uri(hm->uri.p, hm->uri.len);

   try {
      std::vector<std::string> tokens = string_util::split(uri, "[/]+");

      if (tokens.size() <= 1) {
         mg_serve_http(conn, hm, Server::getInstance().http_server_opts);

      } else if (tokens[1] == "rest" && tokens.size() >= 4) {
         if (tokens[2] == "schema") {
            printJson(conn, NDSInstances::getInstance().schema(tokens[3]), 200);
         } else if (tokens.size() >= 5 && tokens[2] == "query") {
            printJson(conn, NDSInstances::getInstance().query(Query(tokens)), 200);
         } else {
            printJson(conn, "[]", 200);
         }
      } else {
         mg_serve_http(conn, hm, Server::getInstance().http_server_opts);
      }
   } catch (...) {
      std::cout << uri << std::endl;
      printJson(conn, "[]", 200);
   }
}

void Server::printText(mg_connection* conn, const std::string& content, int code) {
   static const std::string sep = "\r\n";

   std::stringstream ss;
   ss << "HTTP/1.1 " << code << " OK" << sep
      << "Content-Type: text/plain" << sep
      << "Access-Control-Allow-Origin: " << "*" << sep
      << "Connection: keep-alive" << sep;

      if (Server::getInstance().nds_opts.cache)
         ss << "Cache-Control: public, max-age=" << "86400" << sep;

      ss << "Content-Length: %d" << sep << sep
      << "%s";

   mg_printf(conn, ss.str().c_str(), (int)content.size(), content.c_str());
}

void Server::printJson(mg_connection* conn, const std::string& content, int code) {
   static const std::string sep = "\r\n";

   std::stringstream ss;
   ss << "HTTP/1.1 " << code << " OK" << sep
      << "Content-Type: application/json" << sep
      << "Access-Control-Allow-Origin: " << "*" << sep
      << "Connection: keep-alive" << sep;

      if (Server::getInstance().nds_opts.cache && content != "[]")
         ss << "Cache-Control: public, max-age=" << "86400" << sep;

      ss << "Content-Length: %d" << sep << sep
      << "%s";

   mg_printf(conn, ss.str().c_str(), (int)content.size(), content.c_str());
}
