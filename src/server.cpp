#include "server.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

Server::Server(int port) : port{port} {}

int load_config() {
    std::ifstream config_file("config.json");
    if(!config_file.is_open()){
        spdlog::error("Failed to open the config.json");
        return 8080;
    }
    nlohmann::json config;
    config_file >> config;
    return config.value("port", 8080);
}

void Server::start() {
    spdlog::info("starting server on port {}", port);
    
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res){
        res.set_content("Server is running!", "text/plain");
    });

    svr.Get("/health", [](const httplib::Request& req, httplib::Response& res){
        res.set_content("API stats", "text/plain");
    });

    svr.Post("/echo", [](const httplib::Request& req, httplib::Response& res){
        res.set_content(req.body, "text/plain");
    });

    svr.Get("/json", [](const httplib::Request& req, httplib::Response& res){
        res.set_content(R"({
            "message": "hello there"
           })", "application/json");
    });
    
    std::cout << "Server listenting on port: " << port << std::endl;
    svr.listen("0.0.0.0", port);
}