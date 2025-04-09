// server/Router.h
#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include <httplib.h>
#include <nlohmann/json.hpp>

class Router {
public:
    void updateRoutes(const nlohmann::json& json);
    void setupRouteHandler(httplib::Server& svr);
    const std::unordered_map<std::string, std::string>& getRoutes() const;
    std::unordered_map<std::string, std::string>& getRoutesMutable();

private:
    std::unordered_map<std::string, std::string> route_map;
    std::mutex mutex;
};