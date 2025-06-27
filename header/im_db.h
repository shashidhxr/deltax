#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

using RouteMap = std::unordered_map<std::string, std::string>;      // todo - replace with DS for more route metadata
using UserRouteMap = std::unordered_map<std::string, RouteMap>;

class InMemoryDB {
private:
    UserRouteMap userRoutes;
    std::mutex mutex;

public:
    InMemoryDB() = default;

    void updateUserRoutes(const std::string& user_id, const RouteMap& routes);
    RouteMap getUserRoutes(const std::string& user_id);
    UserRouteMap getAllRoutes();
};