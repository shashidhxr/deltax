#include "server.h"
#include "auth.h"
#include "config.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <mutex>

// Initialize ConfigManager with the WebSocket URL
ConfigManager configManager("ws://localhost:5000");

Server::Server(int port) : port{port} {}

void Server::start()
{
    // Start the WebSocket listener for config updates
    configManager.start_websocket_listener();
    
    // Initial config fetch from server
    configManager.fetch_config_from_server();
    
    spdlog::info("Starting server on port {}", port);

    svr.Get("/", [](const httplib::Request &req, httplib::Response &res) { 
        res.set_content("Welcome to deltax: API-Gateway", "text/plain"); 
    });

    svr.Get("/.*", [](const httplib::Request &req, httplib::Response &res) {
        // Get the API configuration for this path
        const APIConfig* config = configManager.get_config(req.path);
        if(!config){
            res.status = 404;
            res.set_content("Route not found", "text/plain");
            return;
        }

        std::string client_ip = req.remote_addr.empty() ? "127.0.0.1": req.remote_addr;
        spdlog::info("Forwarding request for path {} to {}", req.path, config->target_url);

        // Forward the request to the target URL
        httplib::Client cli(config->target_url.c_str());
        auto backend_res = cli.Get(req.path.c_str());

        if(backend_res){
            res.status = backend_res->status;
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            res.set_content("Failed to reach backend service", "text/plain");
        } 
    }); 

    // Add support for POST requests
    svr.Post("/.*", [](const httplib::Request &req, httplib::Response &res) {
        const APIConfig* config = configManager.get_config(req.path);
        if(!config){
            res.status = 404;
            res.set_content("Route not found", "text/plain");
            return;
        }

        std::string client_ip = req.remote_addr.empty() ? "127.0.0.1": req.remote_addr;
        spdlog::info("Forwarding POST request for path {} to {}", req.path, config->target_url);

        // Forward the request to the target URL
        httplib::Client cli(config->target_url.c_str());
        auto backend_res = cli.Post(
            req.path.c_str(),
            req.body,
            req.get_header_value("Content-Type")
        );

        if(backend_res){
            res.status = backend_res->status;
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            res.set_content("Failed to reach backend service", "text/plain");
        }
    });

    // Add handlers for other HTTP methods as needed (PUT, DELETE, etc.)

    spdlog::info("Server listening on port: {}", port);
    svr.listen("0.0.0.0", port);
}