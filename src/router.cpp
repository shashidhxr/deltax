#include "router.h"
#include <sstream>
#include <spdlog/spdlog.h>
#include <string>

Router::Router(InMemoryDB& db) : dbRef(db) {}

void Router::setupRouteHandler(httplib::Server& svr) {
    svr.Get("/.*", [this](const httplib::Request& req, httplib::Response& res){
        std::vector<std::string> parts = parsePathParts(req.path);

        if (parts.size() < 2) {
            res.status = 400;
            res.set_content("Invalid path format. Use <gatewayURL>/<userId>/<exposedPath>", "text/plain");
            return;
        }

        const std::string& userId = parts[0];
        const std::string exposedPath = "/" + parts[1];

        std::string target_url;
        if (!resolveTargetURL(userId, exposedPath, target_url)) {
            res.status = 404;
            res.set_content("User or route not found.", "text/plain");
            return;
        }

        spdlog::info("Forwarding req for user: {} path {} to {}", userId, exposedPath, target_url);

        std::string host, target_path;
        sanitizeTargetURL(target_url, host, target_path);

        forwardRequest(host, target_path, res);
    });
}

// helper functions

std::vector<std::string> Router::parsePathParts(const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string segment;

    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            parts.push_back(segment);
        }
    }
    return parts;
}

bool Router::resolveTargetURL(const std::string& userId, const std::string& exposedPath, std::string& targetUrl) {
    RouteMap userRoutes = dbRef.getUserRoutes(userId);
    auto routeIt = userRoutes.find(exposedPath);
    if (routeIt == userRoutes.end()) {
        return false;
    }
    targetUrl = routeIt->second;
    return true;
}

void Router::sanitizeTargetURL(std::string& url, std::string& host, std::string& path) {
    path = "/";
    if (url.rfind("http://", 0) == 0) {     // starts_with
        url = url.substr(7);
    } else if (url.rfind("https://", 0) == 0) {
        url = url.substr(8);
        spdlog::warn("HTTPS might not be fully supported");
    }

    size_t pathPos = url.find('/');
    if (pathPos != std::string::npos) {
        host = url.substr(0, pathPos);
        path = url.substr(pathPos);
    } else {
        host = url;
    }
}

void Router::forwardRequest(const std::string& host, const std::string& path, httplib::Response& res) {
    httplib::Client cli(host.c_str());
    auto backend_res = cli.Get(path.c_str());

    if (backend_res) {
        res.status = backend_res->status;
        res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
    } else {
        res.status = 500;
        spdlog::error("Failed to reach backend at {}{}", host, path);
        res.set_content("Failed to reach backend", "text/plain");
    }
}
