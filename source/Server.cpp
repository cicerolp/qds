#include "stdafx.h"
#include "Server.h"
#include "NDSInstances.h"

#include "AugmentedSeries.h"
#include "Clustering.h"

uint32_t g_Quadtree_Depth{18};
extern uint32_t TIMER_ID;

void Server::run(server_opts opts) {
  Server::getInstance().nds_opts = opts;

  mg_mgr_init(&Server::getInstance().mgr, nullptr);
  Server::getInstance().nc = mg_bind(
      &Server::getInstance().mgr,
      std::to_string(Server::getInstance().nds_opts.port).c_str(), handler);

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

  struct http_message* hm = (struct http_message*)p;
  std::string uri(hm->uri.p, hm->uri.len);

  try {
    std::vector<std::string> tokens = util::split(uri, "[/]+");

    if (tokens.size() <= 1) {
      mg_serve_http(conn, hm, Server::getInstance().http_server_opts);

    } else if (tokens[1] == "api" && tokens.size() >= 4) {

      PRINTCSVF("id", TIMER_ID, "uri", uri)

      if (tokens[2] == "schema") {
        printJson(conn, NDSInstances::getInstance().schema(uri));
      } else if (tokens[2] == "query") {
        printJson(conn, NDSInstances::getInstance().query(Query(uri)));
      } else if (tokens[2] == "pipeline") {
        printJson(conn, NDSInstances::getInstance().pipeline(Pipeline(uri)));
      } else if (tokens[2] == "augmented_series") {
        printJson(conn, NDSInstances::getInstance().augmented_series(AugmentedSeries(uri)));
      } else if (tokens[2] == "clustering") {
        printJson(conn, NDSInstances::getInstance().clustering(Clustering(uri)));
      } else {
        printJson(conn, "[]");
      }

      TIMER_INCR_ID

    } else {
      mg_serve_http(conn, hm, Server::getInstance().http_server_opts);
    }
  } catch (...) {
    std::cout << uri << std::endl;
    printJson(conn, "[]");
  }
}

void Server::printText(mg_connection* conn, const std::string& content) {
  if (Server::getInstance().nds_opts.cache) {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Connection: keep-alive\r\n"
              "Cache-Control: public, max-age=86400\r\n"
              "\r\n"
              "%s",
              (int)content.size(), content.c_str());
  } else {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: %d\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Connection: keep-alive\r\n"
              "Cache-Control: no-cache, no-store, must-revalidate\r\n"
              "\r\n"
              "%s",
              (int)content.size(), content.c_str());
  }
}

void Server::printJson(mg_connection* conn, const std::string& content) {
  if (Server::getInstance().nds_opts.cache && content != "[]") {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Connection: keep-alive\r\n"
              "Cache-Control: public, max-age=86400\r\n"
              "\r\n"
              "%s",
              (int)content.size(), content.c_str());
  } else {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %d\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Connection: keep-alive\r\n"
              "Cache-Control: no-cache, no-store, must-revalidate\r\n"
              "\r\n"
              "%s",
              (int)content.size(), content.c_str());
  }
}
