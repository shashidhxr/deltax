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

ConfigManager configManager("ws://localhost:3000/ws");
Server::Server(int port) : port{port} {}

// std::mutex rate_limit_mutex;

// const int MAX_TOKENS = 10;
// const int REFILL_INTERVAL = 1000;   // in ms

// struct rateLimitData {
//     int tokens = MAX_TOKENS;
//     std::chrono::steady_clock::time_point last_refill_time = std::chrono::steady_clock::now();
// };

// std::unordered_map<std::string, rateLimitData> rate_limit_map;

// bool is_req_allowed(const std::string &ip){
//     std::lock_guard<std::mutex> lock(rate_limit_mutex);

//     auto &rate_data = rate_limit_map[ip];

//     auto now = std::chrono::steady_clock::now();
//     auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - rate_data.last_refill_time).count();

//     int tokens_to_add = elapsed_time / REFILL_INTERVAL;
//     if(tokens_to_add > 0){
//         rate_data.tokens = std::min(rate_data.tokens + tokens_to_add, MAX_TOKENS);
//         rate_data.last_refill_time = now;
//     }

//     if(rate_data.tokens > 0){
//         rate_data.tokens--;
//         return true;
//     } else {
//         return false;
//     }
// }

// int load_config()
// {
//     std::ifstream config_file("../config.json");
//     if (!config_file.is_open())
//     {
//         spdlog::error("Failed to open the config.json");
//         return 8080;
//     }
//     nlohmann::json config;
//     config_file >> config;
//     return config.value("port", 9000);
// }

void Server::start()
{
    configManager.start_websocket_listener();
    spdlog::info("starting server on port {}", port);

    // todo - fetch hosts from a json file
    // std::vector<std::string> user_service_hosts = {"http://localhost:5000", "http://localhost:5001", "http://localhost:5005"};
    // int user_service_index = 0;

    svr.Get("/", [](const httplib::Request &req, httplib::Response &res) { 
        res.set_content("Welcome to deltax: API-Gateway", "text/plain"); 
    });

    // svr.Get("/in/users", [&user_service_index, user_service_hosts](const httplib::Request &req, httplib::Response &res) {
    svr.Get("/.*", [](const httplib::Request &req, httplib::Response &res) {
        const APIConfig* config = configManager.get_config(req.path);
        if(!config){
            res.status = 404;
            res.set_content("Route not found", "text/plain");
            return;
        }

        std::string client_ip = req.remote_addr.empty() ? "127.0.0.1": req.remote_addr;

        // if(!is_req_allowed(client_ip)) {
        //     spdlog::warn("Rate limit exceeded for IP: {}", client_ip);
        //     res.status = 429;
        //     res.set_content("Too many requests", "text/plain");
        //     return;
        // }

        // if(!verify_token(req)) {
        //     res.status = 401;
        //     res.set_content("Unauthorized", "text/plain");
        //     return;
        // }

        // std::cout << "verified: " << verify_token(req) << std::endl;                

        // std::string backend_url = user_service_hosts[user_service_index];
        // user_service_index = (user_service_index + 1) % user_service_hosts.size();
        
        // httplib::Client cli(backend_url.c_str());
        // auto backend_res = cli.Get("/api/users");

        httplib::Client cli(config->target_url.c_str());
        auto backend_res = cli.Get(config->target_url.c_str());

        if(backend_res){
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            res.set_content("Failed to reach user service", "text/plain");
        } 
    }); 

    std::cout << "Server listenting on port: " << port << std::endl;
    svr.listen("0.0.0.0", port);
}