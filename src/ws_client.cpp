// server/WebSocketClient.cpp
#include "ws_client.h"
#include <spdlog/spdlog.h>

WebSocketClient::WebSocketClient(Router& router, ConfigManager& configManager)
    : router(router), configManager(configManager) {}

void WebSocketClient::connect(const std::string& url) {
    spdlog::info("Connecting to WebSocket backend ({}).", url);

    ws.setUrl(url);

    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            spdlog::info("Received config update: {}", msg->str);
            auto json = nlohmann::json::parse(msg->str);
            router.updateRoutes(json);
            configManager.save(router.getRoutes());
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            spdlog::info("WebSocket connection established");
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            spdlog::error("WebSocket error: {}", msg->errorInfo.reason);
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            spdlog::warn("WebSocket connection closed");
        }
    });

    ws.start();
}
