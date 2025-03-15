#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <httplib.h>

struct APIConfig {
    int api_id;
    int user_id;
    std::string exposed_url;
    std::string target_url;
    std::string method;
};

class ConfigManager {
public:
    ConfigManager(const std::string& websocket_url);
    
    void start_websocket_listener();
    const APIConfig* get_config(const std::string& path) const;
    
    // Add these methods to handle config file operations
    void fetch_config_from_server();
    void save_config_to_file();
    void load_config_from_file();

private:
    void update_config(const nlohmann::json& config);
    
    std::string websocket_url_;
    std::string config_file_path_;
    ix::WebSocket ws_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, APIConfig> config_map_;
    nlohmann::json full_config_;  // Store the full config for saving to file
};