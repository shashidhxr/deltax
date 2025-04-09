// server/Router.cpp
#include "router.h"

#include <spdlog/spdlog.h>

void Router::updateRoutes(const nlohmann::json &routesJson) {
    std::lock_guard<std::mutex> lock(mutex);
    route_map.clear();

    for (auto &[key, value] : routesJson.items()) {
        if (!value.is_string()) {
            spdlog::warn("Skipping route {} due to non-string target.", key);
            continue;
        }
        route_map[key] = value.get<std::string>();
    }

    spdlog::info("Updated route configuration with {} routes", route_map.size());
}

void Router::setupRouteHandler(httplib::Server& svr) {
    svr.Get("/.*", [this](const httplib::Request &req, httplib::Response &res) {
        std::string path = req.path;
        std::string target_url;

        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = route_map.find(path);
            if (it == route_map.end()) {
                res.status = 404;
                res.set_content("Route not found", "text/plain");
                return;
            }
            target_url = it->second;
        }

        spdlog::info("Forwarding request for {} to {}", path, target_url);

        std::string host;
        std::string target_path = "/";

        if (target_url.substr(0, 7) == "http://") {
            target_url = target_url.substr(7);
        } else if (target_url.substr(0, 8) == "https://") {
            target_url = target_url.substr(8);
            spdlog::warn("HTTPS might not be fully supported");
        }

        size_t path_pos = target_url.find('/');
        if (path_pos != std::string::npos) {
            host = target_url.substr(0, path_pos);
            target_path = target_url.substr(path_pos);
        } else {
            host = target_url;
        }

        httplib::Client cli(host.c_str());
        auto backend_res = cli.Get(target_path.c_str());

        if (backend_res) {
            res.status = backend_res->status;
            res.set_content(backend_res->body, backend_res->get_header_value("Content-Type"));
        } else {
            res.status = 500;
            spdlog::error("Failed to reach backend at {}{}", host, target_path);
            res.set_content("Failed to reach backend", "text/plain");
        }
    });
}

const std::unordered_map<std::string, std::string>& Router::getRoutes() const {
    return route_map;
}

std::unordered_map<std::string, std::string>& Router::getRoutesMutable() {
    return route_map;
}


