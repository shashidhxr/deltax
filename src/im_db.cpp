#include "im_db.h"

#include <spdlog/spdlog.h>

void InMemoryDB::updateUserRoutes(std::string& user_id, const RouteMap& routes) {
    userRoutes[user_id] = routes;           // bug - overwrites 
    spdlog::info("IMDB: updated {} routes for user {}", routes.size(), user_id);
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