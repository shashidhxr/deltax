#pragma once

#include <string>
#include <unordered_map>
#include <httplib.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <mutex>

class Server {
public:
    Server(int port);
    void start();

private:
    int port;
    std::unordered_map<std::string, std::string> route_map;
    httplib::Server svr;
    ix::WebSocket ws;
    std::mutex config_mutex;
    std::string config_file = "config.json";

    void connectToWebSocket();
    void updateRouteConfig(const std::string &data);
    bool loadConfigFromFile();
    void saveConfigToFile();
    void setupRoutes();
};