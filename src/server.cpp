#include "server.h"
#include "auth.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

Server::Server(int port) : port{port} {}

int load_config()
{
    std::ifstream config_file("../config.json");
    if (!config_file.is_open())
    {
        spdlog::error("Failed to open the config.json");
        return 8080;
    }
    nlohmann::json config;
    config_file >> config;
    return config.value("port", 9000);
}

void Server::start()
{
    spdlog::info("starting server on port {}", port);

    // task - fetch hosts from a json file
    std::vector<std::string> user_service_hosts = {"http://localhost:5000", "http://localhost:5001"};
    int user_service_index = 0;

    svr.Get("/", [](const httplib::Request &req, httplib::Response &res)
            { res.set_content("Welcome to deltax: API-Gateway", "text/plain"); });

    svr.Get("/in/users", [&user_service_index, user_service_hosts](const httplib::Request &req, httplib::Response &res)
            {
        std::string backend_url = user_service_hosts[user_service_index];
        user_service_index = (user_service_index + 1) % user_service_hosts.size();
        
        httplib::Client cli(backend_url.c_str());
        auto backend_res = cli.Get("/api/users");

        if(backend_res){
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            res.set_content("Failed to reach user service", "text/plain");
        } });

    svr.Get("/health", [](const httplib::Request &req, httplib::Response &res)
            { res.set_content("API stats", "text/plain"); });

    svr.Post("/echo", [](const httplib::Request &req, httplib::Response &res)
             { res.set_content(req.body, "text/plain"); });

    svr.Get("/json", [](const httplib::Request &req, httplib::Response &res)
            { res.set_content(R"({
            "message": "hello there"
           })",
                              "application/json"); });

    std::cout << "Server listenting on port: " << port << std::endl;
    svr.listen("0.0.0.0", port);
}