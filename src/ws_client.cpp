#include "ws_client.h"

#include <spdlog/spdlog.h>

WebSocketClient::WebSocketClient(ConfigManager& configManager)
    : configManager(configManager) {}


void WebSocketClient::connect(const std::string& url) {
    spdlog::info("WSClient: Connecting to WS backend ({})", url);
    ws.setUrl(url);

    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        switch(msg->type) {
            case ix::WebSocketMessageType::Message:
                handleMessage(msg->str);
                break;
            case ix::WebSocketMessageType::Open:
                spdlog::info("WS connection established");
                break;
            case ix::WebSocketMessageType::Close:
                spdlog::warn("WS connection closed");
                break;
            case ix::WebSocketMessageType::Error:
                spdlog::error("WS error: {}", msg->errorInfo.reason);
                break;
        }
    });

    ws.start();
}

void WebSocketClient::handleMessage(const std::string& msg) {
    spdlog::info("WSClient: Recieved message via WS: {}", msg);
    try {
        auto json = nlohmann::json::parse(msg);

        // new route or update route msg
        if(json.contains("userId") && json.contains("routes")) {
            processRouteUpdate(json);
        }  
        // todo handle other message - delete route, update exisint route  
    } catch (const std::exception& e){
        spdlog::error("WSClient: Failed to process WS msg: {}", e.what());
    }
}

void WebSocketClient::processRouteUpdate(const nlohmann::json& json) {
    std::string userId = json["userId"];
    nlohmann::json routesObject = nlohmann::json::object();     // new object to store routes

    const auto routes = json["routes"];             
    const auto routeItems = routes.items();

    for (const auto& [path, routeData]: routeItems) {
        if(routeData.is_object() && routeData.contains("target")) {
            routesObject[path] = routeData["target"];
        } else {
            spdlog::warn("Invalid route format at {}, skipping", path);
        }
    }

    if(!routesObject.empty()) {
        // router.updateUserRoutes(userId, routesObject);
        configManager.updateUserRoutes(userId, routesObject);
        spdlog::info("WSClient: Routes updated for {}", userId);
    }
}