#pragma once

#include "config.h"

#include <ixwebsocket/IXWebSocket.h>

class WebSocketClient {
public:
    WebSocketClient(ConfigManager& configManager); 
    void connect(const std::string& url);

private:
    void handleMessage(const std::string& msg);
    void processRouteUpdate(const nlohmann::json& json);
    // void processRouteDeletion(const)

    ix::WebSocket ws;
    ConfigManager& configManager;
};