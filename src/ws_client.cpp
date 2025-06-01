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
            // spdlog::info("Received config update: {}", msg->str);
            spdlog::info("Received message via WS: {}", msg->str);
            
            try {
                auto json = nlohmann::json::parse(msg->str);
                
                // Check if we have userId in the message
                if (json.contains("userId") && json.contains("routes") && json["routes"].is_object()) {
                    std::string userId = json["userId"].get<std::string>();
                    
                    // Extract routes in the format expected by the router
                    nlohmann::json routesJson = nlohmann::json::object();
                    
                    // Process each route
                    for (auto& [path, routeData] : json["routes"].items()) {
                        // Check if the route has a target directly or nested in a target property
                        std::string targetUrl;
                        

                        // todo - structure the conditions better
                        // if (routeData.is_string()) {
                        //     // Simple format: "/path": "http://target.com"
                        //     targetUrl = routeData.get<std::string>();
                        // } 
                        if (routeData.is_object() && routeData.contains("target")) {
                            // Complex format: "/path": { "target": "http://target.com", ... }
                            targetUrl = routeData["target"].get<std::string>();
                            
                            // Log additional config info that we'll use in the future

                            // todo - additional info for routing
                            // if (routeData.contains("config")) {
                            //     spdlog::debug("Route {} has additional config", path);
                            //     // In the future, we can extract and use these configs
                            // }
                        }
                        else {
                            spdlog::warn("Route {} has invalid format, skipping", path);
                            continue;
                        }
                        
                        // Add to our simplified routes format
                        routesJson[path] = targetUrl;
                        std::cout << routesJson << std::endl;
                    }
                    
                    // Update routes for this specific user
                    router.updateUserRoutes(userId, routesJson);
                    
                    // Save only this user's routes to avoid overwriting others
                    configManager.updateUserRoutes(userId, router.getUserRoutes(userId));
                    
                    spdlog::info("Successfully updated routes for user {}", userId);
                } 
                
                
                // todo - handle more user and routes CRUD better
                // Handle user deletion if needed
                // else if (json.contains("userId") && json.contains("action") && 
                //          json["action"].get<std::string>() == "delete") {
                //     std::string userId = json["userId"].get<std::string>();
                //     router.removeUserRoutes(userId);
                    
                //     // Save the current state without this user
                //     configManager.save(router.getAllUserRoutes());
                    
                //     spdlog::info("Removed routes for user {}", userId);
                // }
                else {
                    spdlog::warn("WebSocket message format invalid: requires 'userId' and 'routes' fields");
                }
            }
            catch (const std::exception& e) {
                spdlog::error("Failed to process WebSocket message: {}", e.what());
            }
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


// void WebSocketClient::connect(const std::string& url) {
//     spdlog::info("Connecting to WS backend ({})", url);
//     ws.setUrl(url);

//     ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
//         switch(msg->type) {
//             case ix::WebSocketMessageType::Message:
//                 handleMessage(msg->str);
//                 break;
//             case ix::WebSocketMessageType::Open:
//                 spdlog::info("WS connection established");
//                 break;
//             case ix::WebSocketMessageType::Close:
//                 spdlog::warn("WS connection closed");
//                 break;
//             case ix::WebSocketMessageType::Error:
//                 spdlog::error("WS error: {}", msg->errorInfo.reason);
//                 break;
//         }
//     })

//     ws.start();
// };


void WebSocketClient::handleMessage(const std::string& msg) {
    spdlog::info("Recieved message via WS", msg);
    try {
        auto json = nlohmann::json::parse(msg);

        // new route or update route msg
        if(json.contains("userId") && json.contains("routes")) {
            processRouteUpdate(json);
        }  
        // todo handle other message - delete route, update exisint route  
    } catch (const std::exception& e){
        spdlog::error("Failed to process WS msg: {}", e.what());
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
        router.updateUserRoutes(userId, routesObject);
        configManager.updateUserRoutes(userId, router.getUserRoutes(userId));
        spdlog::info("Routes updated for {}", userId);
    }
}