#ifndef CONFIG_H
#define CONFIG_H

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <ixwebsocket/IXWebSocket.h>

struct APIConfig {
    std::string exposed_url;
    std::string target_url;
    std::string method;
};

class ConfigManager {
public:
    explicit ConfigManager(const std::string& websocket_url);
    void start_websocket_listener();
    void saveConfig(const std::string& filename);
    const APIConfig* get_config(const std::string& path) const;

private:
    void update_config(const nlohmann::json& config);

    std::unordered_map<std::string, APIConfig> config_map_;
    mutable std::mutex mutex_;
    std::string websocket_url_;
    ix::WebSocket ws_;
};

#endif
