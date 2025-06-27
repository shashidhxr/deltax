#include "config.h"

ConfigManager::ConfigManager(InMemoryDB& db) : db(db) {

}

void ConfigManager::loadInitConfig() {
    
}

void ConfigManager::updateUserRoutes(const std::string& user_id, const RouteMap& routes) {
    dbRef.updateUserRoutes(user_id, routes);
}

RouteMap ConfigManager::getUserRoutes(const std::string& user_id) {
    dbRef.getUserRoutes(user_id);
}

UserRouteMap ConfigManager::getAllRoutes() {
    dbRef.getAllRoutes();
}
