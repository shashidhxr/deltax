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
    void handleMessage(const std::string& msg);
    void processRouteUpdate(const nlohmann::json& json);
    // void processRouteDeletion(const)

    ix::WebSocket ws;
    Router& router;
    ConfigManager& configManager;
};