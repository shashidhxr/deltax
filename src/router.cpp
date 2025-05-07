// server/Router.cpp
#include "router.h"
#include "rate_limiter.h"

#include <spdlog/spdlog.h>

// This version of Router.cpp allows storing additional configuration data
// while still maintaining compatibility with the current implementation

struct RouteConfig {
    std::string targetUrl;
    std::string method = "GET";  // Default to GET
    int rateLimit = 0;           // 0 means no limit
    std::string authType;
    bool loadBalancingEnabled = false;
    std::string loadBalancingAlgorithm;
    std::vector<std::string> loadBalancingTargets;
    bool securityCors = false;
    bool securitySsl = false;
    std::vector<std::string> securityIpWhitelist;
};

RateLimiter rateLimiter(5, 10);

// This map will store the extended configuration for future use
static std::unordered_map<std::string, std::unordered_map<std::string, RouteConfig>> routeConfigs;

void Router::updateUserRoutes(const std::string& userId, const nlohmann::json& routesJson) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Create or clear existing routes for this user
    RouteMap& routes = user_routes[userId];
    routes.clear();
    
    // Also clear extended configs
    routeConfigs[userId].clear();
    
    // Add all routes from the JSON
    for (auto& [path, value] : routesJson.items()) {
        std::string targetUrl;
        RouteConfig config;
        
        if (value.is_string()) {
            // Simple case: just a target URL
            targetUrl = value.get<std::string>();
            config.targetUrl = targetUrl;
        } 
        else if (value.is_object()) {
            // Complex case: object with target and config
            if (value.contains("target") && value["target"].is_string()) {
                targetUrl = value["target"].get<std::string>();
                config.targetUrl = targetUrl;
                
                // Extract additional config parameters if available
                if (value.contains("method") && value["method"].is_string()) {
                    config.method = value["method"].get<std::string>();
                }
                
                if (value.contains("config") && value["config"].is_object()) {
                    auto& configObj = value["config"];
                    
                    // Rate limiting
                    if (configObj.contains("rateLimit") && configObj["rateLimit"].is_number()) {
                        config.rateLimit = configObj["rateLimit"].get<int>();
                    }
                    
                    // Authentication
                    if (configObj.contains("auth") && configObj["auth"].is_string()) {
                        config.authType = configObj["auth"].get<std::string>();
                    }
                    
                    // Load balancing
                    if (configObj.contains("loadBalancing") && configObj["loadBalancing"].is_object()) {
                        auto& lb = configObj["loadBalancing"];
                        
                        if (lb.contains("enabled") && lb["enabled"].is_boolean()) {
                            config.loadBalancingEnabled = lb["enabled"].get<bool>();
                        }
                        
                        if (lb.contains("algorithm") && lb["algorithm"].is_string()) {
                            config.loadBalancingAlgorithm = lb["algorithm"].get<std::string>();
                        }
                        
                        if (lb.contains("targets") && lb["targets"].is_array()) {
                            for (auto& target : lb["targets"]) {
                                if (target.is_string()) {
                                    config.loadBalancingTargets.push_back(target.get<std::string>());
                                }
                            }
                        }
                    }
                    
                    // Security
                    if (configObj.contains("security") && configObj["security"].is_object()) {
                        auto& sec = configObj["security"];
                        
                        if (sec.contains("cors") && sec["cors"].is_boolean()) {
                            config.securityCors = sec["cors"].get<bool>();
                        }
                        
                        if (sec.contains("ssl") && sec["ssl"].is_boolean()) {
                            config.securitySsl = sec["ssl"].get<bool>();
                        }
                        
                        if (sec.contains("ipWhitelist") && sec["ipWhitelist"].is_array()) {
                            for (auto& ip : sec["ipWhitelist"]) {
                                if (ip.is_string()) {
                                    config.securityIpWhitelist.push_back(ip.get<std::string>());
                                }
                            }
                        }
                    }
                }
            } else {
                spdlog::warn("Skipping route {} due to missing or invalid target.", path);
                continue;
            }
        } else {
            spdlog::warn("Skipping route {} due to invalid value type.", path);
            continue;
        }
        
        // Add the route to our current simple map (path -> targetUrl)
        routes[path] = targetUrl;
        
        // Store the extended config for future use
        routeConfigs[userId][path] = config;
    }
    
    spdlog::info("Updated routes for user {}: {} routes configured", userId, routes.size());
}

void Router::removeUserRoutes(const std::string& userId) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = user_routes.find(userId);
    if (it != user_routes.end()) {
        user_routes.erase(it);
        // Also remove extended configs
        routeConfigs.erase(userId);
        spdlog::info("Removed all routes for user {}", userId);
    }
}

// The rest of the functions remain the same as in your previous implementation
void Router::setupRouteHandler(httplib::Server& svr) {
    svr.Get("/.*", [this](const httplib::Request& req, httplib::Response& res){
        std::string path = req.path;
        std::vector<std::string> parts;
        std::stringstream ss(path); 
        std::string segment;

        if (!rateLimiter.allow_req(req.remote_addr)) {
            res.status = 429;
            res.set_content("Rate limit exceeded", "text/plain");
            spdlog::warn("Rate limit exceeded. Request blocked");
            return;
        }

        while(std::getline(ss, segment, '/')) {
            if(!segment.empty()){
                parts.push_back(segment);
            }
        }

        if(parts.size() < 2){
            res.status = 400;
            res.set_content("Invalid path format. Use <gatewayURL>/<userId>/<exposedPath>", "text/plain");
            return;
        }

        std::string userId = parts[0];
        std::string exposedPath = "/" + parts[1];

        std::string target_url;
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto user_it = user_routes.find(userId);
            if(user_it == user_routes.end()) {
                res.status = 404;
                res.set_content("User not found. Check your dashboard for the route", "text/plain");
                return;
            }
            auto& routes = user_it->second;         // why & routes
            auto route_it = routes.find(exposedPath);
            if(route_it == routes.end()){
                res.status = 404;
                res.set_content("Route not found. Check your dashboard for the route", "text/plain");
                return;
            }
            target_url = route_it->second;
        }

        // req forwarding
        spdlog::info("Forwarding req for user: {} path {} to {}", userId, exposedPath, target_url);
        
        std::string host;
        std::string target_path = "/";
        if(target_url.substr(0, 7) == "http://"){
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

const UserRouteMap& Router::getAllUserRoutes() const {
    return user_routes;
}

UserRouteMap& Router::getAllUserRoutesMutable() {
    return user_routes;
}

const RouteMap& Router::getUserRoutes(const std::string& userId) const {
    static const RouteMap emptyMap;
    auto it = user_routes.find(userId);
    return (it != user_routes.end()) ? it->second : emptyMap;
}

bool Router::hasUser(const std::string& userId) const {
    return user_routes.find(userId) != user_routes.end();
}