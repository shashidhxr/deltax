#pragma once

#include "router.h"
#include "config.h"
#include "ws_client.h"

#include <httplib.h>

class Server {
public:
    Server(int port, const std::string& config_path);
    void start();

private:
    int port;
    httplib::Server svr;
    Router router;
    ConfigManager configManager;
    WebSocketClient wsClient;
};