#include "im_db.h"

#include <spdlog/spdlog.h>

void InMemoryDB::updateUserRoutes(const std::string& user_id, const RouteMap& routes) {
    if(userRoutes.find(user_id) == userRoutes.end()) {
        spdlog::info("IMDB: New user created {}, with {} routes", user_id, routes.size());
    }
    
    RouteMap& existingRoutes = userRoutes[user_id];
    for (const auto& [path, target] : routes) {
        existingRoutes[path] = target;
    }
    spdlog::info("IMDB: merged {} routes for user {}", routes.size(), user_id);
}

RouteMap InMemoryDB::getUserRoutes(const std::string& user_id) {
    auto it = userRoutes.find(user_id);
    if(it != userRoutes.end()) {
        return it->second;
    } else {
        return {};
    }
}

UserRouteMap InMemoryDB::getAllRoutes() {
    return userRoutes;
}