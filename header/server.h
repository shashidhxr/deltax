#pragma once

#include "ws_client.h"
#include "router.h"
#include "config.h"

#include <httplib.h>

class Server {                              // scale - avoid tight coupling, dependecy injection
public:
    Server(int port);
    void start();

private:
    int port;
    httplib::Server svr;
    InMemoryDB db;
    ConfigManager configManager;        // rf1- replace with inmemdb or cahnge configmanager to use inmenerydb
    Router router;
    WebSocketClient wsClient;
};