#include "server.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <iostream>
#include <httplib.h>

// Cookie credentials - you should load these from a secure configuration
std::string auth_cookie_name = "authToken";
std::string auth_cookie_value = "authToken=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOjUsImVtYWlsIjoidGVzdDlAZ21haWwuY29tIiwiaWF0IjoxNzQ0MTkyOTY1LCJleHAiOjE3NDQyNzkzNjV9.7PZC4bLtyJWMHYMX97Ji5YLvxM715a4a7wDlYFWcZBs; Path=/; HttpOnly; Expires=Thu, 10 Apr 2025 10:02:45 GMT;"; // Replace with your actual token

Server::Server(int port) : port(port) {
    // Try to load config file on startup
    if (!loadConfigFromFile()) {
        spdlog::info("Creating empty config file");
        // Create an empty config file with a valid JSON structure
        nlohmann::json initial_config = {{"routes", nlohmann::json::object()}};
        std::ofstream file(config_file);
        file << initial_config.dump(4);
    }
    
    // Authenticate with the backend server
    authenticateWithBackend();
}

bool Server::loadConfigFromFile() {
    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            spdlog::warn("Config file not found at '{}'", config_file);
            return false;
        }
        
        // Check if file is empty
        file.seekg(0, std::ios::end);
        if (file.tellg() == 0) {
            spdlog::warn("Config file is empty");
            return false;
        }
        
        // Reset file position to beginning
        file.seekg(0, std::ios::beg);
        
        nlohmann::json json;
        try {
            file >> json;
            
            // Verify the JSON has the expected structure
            if (!json.contains("routes") || !json["routes"].is_object()) {
                spdlog::error("Config file has invalid format (missing 'routes' object)");
                return false;
            }
            
            std::lock_guard<std::mutex> lock(config_mutex);
            route_map.clear();
            for (auto &[key, value] : json["routes"].items()) {
                if (value.is_string()) {
                    route_map[key] = value.get<std::string>();
                    spdlog::debug("Loaded route: {} -> {}", key, value.get<std::string>());
                }
            }
            spdlog::info("Loaded configuration from file with {} routes", route_map.size());
            return true;
        } catch (const nlohmann::json::exception& e) {
            spdlog::error("Failed to parse config file JSON: {}", e.what());
            return false;
        }
    } catch (const std::exception& e) {
        spdlog::error("Error accessing config file: {}", e.what());
        return false;
    }
}

void Server::saveConfigToFile() {
    try {
        nlohmann::json json;
        nlohmann::json routes = nlohmann::json::object();
        
        {
            std::lock_guard<std::mutex> lock(config_mutex);
            for (const auto& [path, target] : route_map) {
                routes[path] = target;
            }
        }
        
        json["routes"] = routes;
        
        // Debug output of what we're about to save
        spdlog::debug("Saving config with {} routes: {}", routes.size(), json.dump());
        
        // Use atomic write pattern to prevent corruption
        std::string temp_file = config_file + ".tmp";
        {
            std::ofstream file(temp_file);
            if (!file.is_open()) {
                spdlog::error("Failed to open temp config file for writing: {}", temp_file);
                return;
            }
            file << json.dump(4); // Pretty print with 4-space indent
            file.flush();
            file.close();
            
            if (file.fail()) {
                spdlog::error("Failed to write to temp config file");
                return;
            }
        }
        
        // Rename temp file to actual config file (atomic operation)
        if (std::rename(temp_file.c_str(), config_file.c_str()) != 0) {
            spdlog::error("Failed to rename temp config file to actual config file");
            return;
        }
        
        spdlog::info("Configuration saved to file with {} routes", routes.size());
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config file: {}", e.what());
    }
}

bool Server::authenticateWithBackend() {
    spdlog::info("Authenticating with backend server...");
    
    httplib::Client cli("localhost", 5000);
    
    // Add your authentication logic here
    // If you need to perform a login request first:
    /*
    httplib::Params params;
    params.emplace("username", "your_username");
    params.emplace("password", "your_password");
    
    auto res = cli.Post("/login", params);
    if (!res || res->status != 200) {
        spdlog::error("Authentication failed with status: {}", 
                     res ? std::to_string(res->status) : "connection error");
        return false;
    }
    
    // Extract and store cookies from response
    for (const auto& header : res->headers) {
        if (header.first == "Set-Cookie") {
            session_cookies = header.second;
            break;
        }
    }
    */
    
    // For testing, we'll just set a predefined cookie
    session_cookies = auth_cookie_name + "=" + auth_cookie_value;
    spdlog::info("Authentication successful");
    return true;
}

void Server::connectToWebSocket() {
    spdlog::info("Connecting to WebSocket backend (ws://localhost:5000)...");

    ws.setUrl("ws://localhost:5000");
    
    // Add authentication headers or cookies to WebSocket connection
    ix::WebSocketHttpHeaders headers;
    headers["Cookie"] = session_cookies;
    ws.setExtraHeaders(headers);

    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            spdlog::info("Received data from WebSocket: {}", msg->str);
            
            try {
                auto json = nlohmann::json::parse(msg->str);
                
                // Check if this is an operation notification
                if (json.contains("type") && json["type"] == "config_update" && 
                    json.contains("operation") && json.contains("apiId")) {
                    
                    int api_id = json["apiId"].get<int>();
                    std::string operation = json["operation"].get<std::string>();
                    
                    spdlog::info("Received config_update notification: {} operation for API ID {}", 
                                 operation, api_id);
                    
                    // For create/update operations, fetch the API details
                    if (operation == "create" || operation == "update") {
                        fetchApiConfig(api_id);
                    }
                    // For delete operations, remove the API from our routes
                    else if (operation == "delete") {
                        removeApiConfig(api_id);
                    }
                } 
                // Check if this is a direct routes update (fallback to original implementation)
                else if (json.contains("routes") && json["routes"].is_object()) {
                    if (updateRouteConfig(json)) {
                        saveConfigToFile();
                    }
                }
                else {
                    spdlog::warn("Unrecognized WebSocket message format");
                }
            } catch (const nlohmann::json::exception& e) {
                spdlog::error("Failed to parse WebSocket message: {}", e.what());
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

void Server::fetchApiConfig(int api_id) {
    spdlog::info("Fetching API configuration for ID {}", api_id);
    
    // Make a HTTP request to the Node.js backend to get complete API details
    httplib::Client cli("localhost", 5000);
    httplib::Headers headers = {
        {"Cookie", session_cookies}
    };
    
    std::string endpoint = "/api/in/api/" + std::to_string(api_id);
    auto res = cli.Get(endpoint.c_str(), headers);
    
    if (!res) {
        spdlog::error("Failed to fetch API configuration for ID {}", api_id);
        return;
    }
    
    if (res->status == 401) {
        spdlog::error("Authentication failed when fetching API ID {}", api_id);
        // Try to re-authenticate
        if (authenticateWithBackend()) {
            // Retry with new credentials
            headers = {{"Cookie", session_cookies}};
            res = cli.Get(endpoint.c_str(), headers);
            if (!res || res->status != 200) {
                spdlog::error("Retry failed with status: {}", 
                             res ? std::to_string(res->status) : "connection error");
                return;
            }
        } else {
            return;
        }
    } else if (res->status != 200) {
        spdlog::error("API server returned status {} when fetching API ID {}", 
                      res->status, api_id);
        return;
    }
    
    spdlog::debug("API server response: {}", res->body);
    
    try {
        auto json = nlohmann::json::parse(res->body);
        
        // Extract API details and update route map
        if (json.contains("path") && json.contains("targetUrl")) {
            std::string path = json["path"].get<std::string>();
            std::string target_url = json["targetUrl"].get<std::string>();
            
            spdlog::info("Adding/updating route: {} -> {}", path, target_url);
            
            {
                std::lock_guard<std::mutex> lock(config_mutex);
                route_map[path] = target_url;
                // Also store the mapping of api_id to path for future reference
                api_to_path_map[api_id] = path;
            }
            
            // Save the updated configuration
            saveConfigToFile();
        } else {
            spdlog::error("API configuration missing required fields");
        }
    } catch (const nlohmann::json::exception& e) {
        spdlog::error("Failed to parse API configuration: {}", e.what());
    }
}

void Server::removeApiConfig(int api_id) {
    spdlog::info("Removing API configuration for ID {}", api_id);
    
    std::string path_to_remove;
    
    {
        std::lock_guard<std::mutex> lock(config_mutex);
        auto it = api_to_path_map.find(api_id);
        if (it == api_to_path_map.end()) {
            spdlog::warn("API ID {} not found in mapping", api_id);
            return;
        }
        
        path_to_remove = it->second;
        api_to_path_map.erase(it);
        
        auto route_it = route_map.find(path_to_remove);
        if (route_it != route_map.end()) {
            spdlog::info("Removing route: {}", path_to_remove);
            route_map.erase(route_it);
        }
    }
    
    // Save the updated configuration
    saveConfigToFile();
}

bool Server::updateRouteConfig(const nlohmann::json &json) {
    try {
        if (!json.contains("routes") || !json["routes"].is_object()) {
            spdlog::warn("JSON does not contain a valid routes object");
            return false;
        }
        
        std::lock_guard<std::mutex> lock(config_mutex);
        route_map.clear();
        
        int route_count = 0;
        for (auto &[key, value] : json["routes"].items()) {
            if (value.is_string()) {
                route_map[key] = value.get<std::string>();
                route_count++;
                spdlog::debug("Updated route: {} -> {}", key, value.get<std::string>());
            }
        }
        
        spdlog::info("Updated route configuration with {} routes", route_count);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error updating route config: {}", e.what());
        return false;
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
                spdlog::warn("Route not found: {}", path);
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

        // Basic URL parsing
        if (target_url.substr(0, 7) == "http://") {
            target_url = target_url.substr(7);
        } else if (target_url.substr(0, 8) == "https://") {
            target_url = target_url.substr(8);
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
        
        // Forward the request with its original headers
        httplib::Headers headers;
        for (const auto& header : req.headers) {
            // Skip host header as we're changing the destination
            if (header.first != "Host") {
                headers.emplace(header.first, header.second);
            }
        }
        
        // Forward the request with the same method and path
        auto backend_res = cli.Get(target_path.c_str(), headers);

        if (backend_res) {
            // Copy status and body
            res.status = backend_res->status;
            res.body = backend_res->body;
            
            // Copy all headers from the backend response
            for (const auto& header : backend_res->headers) {
                res.set_header(header.first.c_str(), header.second);
            }
            
            spdlog::info("Successfully forwarded request and received response with status {}", backend_res->status);
        } else {
            res.status = 500;
            spdlog::error("Failed to reach backend at {}{}", host, target_path);
            res.set_content("Failed to reach backend", "text/plain");
        }
    });
}

void Server::start() {
    // First authenticate with the backend
    if (!authenticateWithBackend()) {
        spdlog::error("Failed to authenticate with backend, exiting");
        return;
    }
    
    connectToWebSocket();
    setupRoutes();

    spdlog::info("API Gateway listening on port {}", port);
    svr.listen("0.0.0.0", port);
}