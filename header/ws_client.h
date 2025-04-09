// server/WebSocketClient.h
#pragma once

#include "router.h"
#include "config.h"

#include <ixwebsocket/IXWebSocket.h>

class WebSocketClient {
public:
    WebSocketClient(Router& router, ConfigManager& configManager);
    void connect(const std::string& url);

private:
    ix::WebSocket ws;
    Router& router;
    ConfigManager& configManager;
};