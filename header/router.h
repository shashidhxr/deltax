// server/Router.h
#pragma once

#include "im_db.h"

#include <string>
#include <httplib.h>

// #include "../vcpkg-headers/httplib.h"
// #include "../vcpkg-headers/nlohmann/json.hpp"

class Router {
private:
    InMemoryDB& dbRef;          // ref to existing db object

public:
    Router(InMemoryDB& db);         // dependency injection, no state duplication

    void setupRouteHandler(httplib::Server& svr);
    
    RouteMap getUserRoutes(const std::string& user_id);
    UserRouteMap getAllRoutes();

    // helper functions
    std::vector<std::string> parsePathParts(const std::string& path);
    bool resolveTargetURL(const std::string& userId, const std::string& exposedPath, std::string& targetUrl);
    void sanitizeTargetURL(std::string& url, std::string& host, std::string& path);
    void forwardRequest(const std::string& host, const std::string& path, httplib::Response& res);
};