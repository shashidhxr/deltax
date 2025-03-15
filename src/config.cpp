#include "config.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <fstream>

ConfigManager::ConfigManager(const std::string& websocket_url) 
    : websocket_url_(websocket_url), config_file_path_("config.json") {
    // Try to load existing config file on startup
    load_config_from_file();
}

void ConfigManager::start_websocket_listener() {
    ws_.setUrl(websocket_url_);
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            spdlog::info("Received WebSocket message: {}", msg->str);
            try {
                nlohmann::json message = nlohmann::json::parse(msg->str);
                
                // Check if this is a config update notification
                if (message.contains("type") && message["type"] == "config_update") {
                    spdlog::info("Config update notification received, fetching new configuration");
                    fetch_config_from_server();
                }
            } catch (const std::exception& e) {
                spdlog::error("Error parsing WebSocket message: {}", e.what());
            }
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            spdlog::info("Connected to WebSocket for config updates");
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            spdlog::warn("WebSocket connection closed. Will attempt to reconnect...");
            // Reconnect logic can be added here
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            spdlog::error("WebSocket error: {}", msg->errorInfo.reason);
        }
    });

    ws_.start();
    spdlog::info("WebSocket client started");
}

void ConfigManager::fetch_config_from_server() {
    spdlog::info("Fetching configuration from server");
    
    // Create HTTP client to fetch the config
    httplib::Client cli("http://localhost:5000"); // Adjust URL to match your Node.js server
    auto res = cli.Get("/api/gateway/config");
    
    if (res && res->status == 200) {
        try {
            nlohmann::json config = nlohmann::json::parse(res->body);
            update_config(config);
            save_config_to_file();
            spdlog::info("Configuration updated successfully");
        } catch (const std::exception& e) {
            spdlog::error("Error parsing config response: {}", e.what());
        }
    } else {
        spdlog::error("Failed to fetch configuration: {}", 
                      res ? std::to_string(res->status) : "connection failed");
    }
}

void ConfigManager::update_config(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_map_.clear();
    
    // Store the full config for saving to file
    full_config_ = config;
    
    // Update the runtime config map
    if (config.contains("apis") && config["apis"].is_array()) {
        for (const auto& api : config["apis"]) {
            APIConfig cfg;
            cfg.api_id = api.value("id", 0);
            cfg.user_id = api.value("user_id", 0);
            cfg.exposed_url = api.value("exposed_url", "");
            cfg.target_url = api.value("target_url", "");
            cfg.method = api.value("method", "GET");
            
            // Add to the config map - use the exposed_url as the key
            if (!cfg.exposed_url.empty()) {
                config_map_[cfg.exposed_url] = cfg;
                spdlog::debug("Added API route: {} -> {}", cfg.exposed_url, cfg.target_url);
            }
        }
    }
}

void ConfigManager::save_config_to_file() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream file(config_file_path_);
    if (file.is_open()) {
        file << full_config_.dump(4);  // Pretty-print with 4 spaces
        file.close();
        spdlog::info("Configuration saved to {}", config_file_path_);
    } else {
        spdlog::error("Failed to open config file for writing: {}", config_file_path_);
    }
}

void ConfigManager::load_config_from_file() {
    std::ifstream file(config_file_path_);
    if (file.is_open()) {
        try {
            nlohmann::json config;
            file >> config;
            update_config(config);
            spdlog::info("Configuration loaded from file: {}", config_file_path_);
        } catch (const std::exception& e) {
            spdlog::error("Error parsing config file: {}", e.what());
        }
        file.close();
    } else {
        spdlog::warn("No existing config file found at {}", config_file_path_);
    }
}

const APIConfig* ConfigManager::get_config(const std::string& path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = config_map_.find(path);
    return it != config_map_.end() ? &it->second : nullptr;
}