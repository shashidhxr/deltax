#include "server.h"
#include <spdlog/spdlog.h>

Server::Server(int port) : port(port) {
    // Try to load config file on startup
    loadConfigFromFile();
}

bool Server::loadConfigFromFile() {
    try {
        std::ifstream file(config_file);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;
            
            std::lock_guard<std::mutex> lock(config_mutex);
            route_map.clear();
            for (auto &[key, value] : json["routes"].items()) {
                route_map[key] = value;
            }
            spdlog::info("Loaded configuration from file with {} routes", route_map.size());
            return true;
        }
        spdlog::warn("Config file not found, starting with empty configuration");
        return false;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config file: {}", e.what());
        return false;
    }
}

void Server::saveConfigToFile() {
    try {
        nlohmann::json json;
        
        {
            std::lock_guard<std::mutex> lock(config_mutex);
            nlohmann::json routes;
            for (const auto& [path, target] : route_map) {
                routes[path] = target;
            }
            json["routes"] = routes;
        }
        
        std::ofstream file(config_file);
        file << json.dump(4); // Pretty print with 4-space indent
        spdlog::info("Configuration saved to file");
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config file: {}", e.what());
    }
}

void Server::connectToWebSocket() {
    spdlog::info("Connecting to WebSocket backend (ws://localhost:5000)...");

    ws.setUrl("ws://localhost:5000");

    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            spdlog::info("Received config update: {}", msg->str);
            std::cout << msg->str << std::endl;
            updateRouteConfig(msg->str);
            saveConfigToFile(); // Save updated config to file
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

void Server::updateRouteConfig(const std::string &data) {
    try {
        auto json = nlohmann::json::parse(data);
        
        std::lock_guard<std::mutex> lock(config_mutex);
        route_map.clear();
        for (auto &[key, value] : json["routes"].items()) {
            route_map[key] = value;
        }
        spdlog::info("Updated route configuration with {} routes", route_map.size());
    } catch (const std::exception& e) {
        spdlog::error("Failed to parse WebSocket config update: {}", e.what());
    }
}

void Server::setupRoutes() {
    // Handle all GET requests
    svr.Get("/.*", [this](const httplib::Request &req, httplib::Response &res) {
        std::string path = req.path;
        std::string target_url;
        
        {
            std::lock_guard<std::mutex> lock(config_mutex);
            auto it = route_map.find(path);
            if (it == route_map.end()) {
                res.status = 404;
                res.set_content("Route not found", "text/plain");
                return;
            }
            target_url = it->second;
        }
        
        spdlog::info("Forwarding request for {} to {}", path, target_url);

        // Parse target URL to get host and path
        std::string host;
        std::string target_path = "/";

        // Basic URL parsing (you might want a more robust solution)
        if (target_url.substr(0, 7) == "http://") {
            target_url = target_url.substr(7);
        } else if (target_url.substr(0, 8) == "https://") {
            target_url = target_url.substr(8);
            // Note: your version of httplib might not fully support HTTPS
            spdlog::warn("HTTPS might not be fully supported with your httplib version");
        }

        size_t path_pos = target_url.find('/');
        if (path_pos != std::string::npos) {
            host = target_url.substr(0, path_pos);
            target_path = target_url.substr(path_pos);
        } else {
            host = target_url;
        }

        // Create a client to forward the request
        httplib::Client cli(host.c_str());
        
        // Forward the request with the same method and path
        auto backend_res = cli.Get(target_path.c_str());

        if (backend_res) {
            res.status = backend_res->status;
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            spdlog::error("Failed to reach backend at {}{}", host, target_path);
            res.set_content("Failed to reach backend", "text/plain");
        }
    });
}

void Server::start() {
    connectToWebSocket();
    setupRoutes();

    spdlog::info("API Gateway listening on port {}", port);
    svr.listen("0.0.0.0", port);
}