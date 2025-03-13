#include "config.h"
#include <spdlog/spdlog.h>
#include <iostream>

ConfigManager::ConfigManager(const std::string& websocket_url) : websocket_url_(websocket_url) {}

void ConfigManager::start_websocket_listener() {
    ws_.setUrl(websocket_url_);
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        std::cout << "ws triggered" << std::endl;
        if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                nlohmann::json config = nlohmann::json::parse(msg->str);
                update_config(config);
                spdlog::info("Received updated API configuration.");
            } catch (const std::exception& e) {
                spdlog::error("Error parsing config JSON: {}", e.what());
            }
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            spdlog::info("Connected to WebSocket for config updates.");
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            spdlog::warn("WebSocket connection closed.");
        }
    });

    ws_.start();
}

void ConfigManager::update_config(const nlohmann::json& config) {
    std::cout << "update triggered via ws" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    config_map_.clear();

    for (const auto& api : config["apis"]) {
        APIConfig cfg;
        cfg.exposed_url = api["exposed_url"];
        cfg.target_url = api["target_url"];
        cfg.method = api["method"];
        config_map_[cfg.exposed_url] = cfg;
    }
}

// void ConfigManger::saveConfig(const std::string& filename) {
//     nlohmann::json config;

//     // Example config data
//     config["server"] = "localhost";
//     config["port"] = 8080;
//     config["use_ssl"] = true;

//     std::ofstream file(filename);
//     if (file.is_open()) {
//         file << config.dump(4);  // Pretty-print with 4 spaces
//         file.close();
//     } else {
//         std::cerr << "Failed to open config file for writing!" << std::endl;
//     }
// }


const APIConfig* ConfigManager::get_config(const std::string& path) const {
    std::cout << "get config triggered via ws" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_map_.find(path);
    return it != config_map_.end() ? &it->second : nullptr;
}
