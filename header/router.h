// server/Router.h
#pragma once

#include "rate_limiter.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <httplib.h>
#include <nlohmann/json.hpp>


using RouteMap = std::unordered_map<std::string, std::string>;
using UserRouteMap = std::unordered_map<std::string, RouteMap>;

class Router {
private:
    UserRouteMap user_routes;  // userId -> {route -> target}
    std::mutex mutex;
    RateLimiter rateLimiter{5, 10};

public:
    Router() = default;

    void updateUserRoutes(const std::string& userId, const nlohmann::json& routesJson);
    void removeUserRoutes(const std::string& userId);
    void setupRouteHandler(httplib::Server& svr);
    const UserRouteMap& getAllUserRoutes() const;
    UserRouteMap& getAllUserRoutesMutable();
    
    const RouteMap& getUserRoutes(const std::string& userId) const;
    bool hasUser(const std::string& userId) const;
};