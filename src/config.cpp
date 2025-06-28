#include "config.h"

#include <spdlog/spdlog.h>

ConfigManager::ConfigManager(InMemoryDB& db) : dbRef(db) {}

void ConfigManager::loadInitConfig() {
    spdlog::info("CFG_Man: Starting with empty In Memory config");

    // todo - cold start logic
}

void ConfigManager::updateUserRoutes(const std::string& user_id, const RouteMap& routes) {
    dbRef.updateUserRoutes(user_id, routes);
    spdlog::info("CFG_Man: Updated routes for user {}", user_id);
}

RouteMap ConfigManager::getUserRoutes(const std::string& user_id) {
    dbRef.getUserRoutes(user_id);
}       

UserRouteMap ConfigManager::getAllRoutes() {
    dbRef.getAllRoutes();
}
