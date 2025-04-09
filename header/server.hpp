#pragma once

#include <string>
#include <unordered_map>
#include <httplib.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <mutex>

class Server {
public:
    Server(int port);
    void start();

private:
    int port;
    std::unordered_map<std::string, std::string> route_map;
    std::unordered_map<int, std::string> api_to_path_map; // Maps API IDs to paths
    httplib::Server svr;
    ix::WebSocket ws;
    std::mutex config_mutex;
    std::string config_file = "config.json";
    std::string session_cookies; // Stores auth cookies

    bool authenticateWithBackend();
    void connectToWebSocket();
    bool loadConfigFromFile();
    void saveConfigToFile();
    bool updateRouteConfig(const nlohmann::json &json);
    void fetchApiConfig(int api_id);
    void removeApiConfig(int api_id);
    void setupRoutes();
};