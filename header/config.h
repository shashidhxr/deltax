#pragma once

#include "im_db.h"

class ConfigManager {
private:
    InMemoryDB dbRef;

public:
    ConfigManager(InMemoryDB& db);

    void updateUserRoutes(const std::string& user_id, const RouteMap& routes);
    void loadInitConfig();          //  cold start
    RouteMap getUserRoutes(const std::string& user_id);
    UserRouteMap getAllRoutes();
};