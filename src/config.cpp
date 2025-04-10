// server/ConfigManager.cpp
#include "config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

ConfigManager::ConfigManager(const std::string& path) : config_file(path) {}

bool ConfigManager::load(UserRouteMap& userRoutes) {
    try {
        std::ifstream file(config_file);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;

            userRoutes.clear();
            
            // Check if we have users object in the JSON
            if (json.contains("users") && json["users"].is_object()) {
                for (auto& [userId, userData] : json["users"].items()) {
                    if (userData.contains("routes") && userData["routes"].is_object()) {
                        RouteMap& routes = userRoutes[userId];
                        for (auto& [path, target] : userData["routes"].items()) {
                            if (target.is_string()) {
                                routes[path] = target.get<std::string>();
                            }
                        }
                        spdlog::info("Loaded {} routes for user {}", routes.size(), userId);
                    }
                }
                spdlog::info("Loaded configuration for {} users", userRoutes.size());
            } else {
                // For backwards compatibility, look for routes at the top level
                if (json.contains("routes") && json["routes"].is_object()) {
                    // If we find old format, migrate it to a default user
                    const std::string defaultUserId = "default";
                    RouteMap& routes = userRoutes[defaultUserId];
                    
                    for (auto& [path, target] : json["routes"].items()) {
                        if (target.is_string()) {
                            routes[path] = target.get<std::string>();
                        }
                    }
                    spdlog::info("Migrated {} routes from old format to default user", routes.size());
                }
            }
            
            return true;
        }
        spdlog::warn("Config file not found, starting with empty configuration");
        return false;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config file: {}", e.what());
        return false;
    }
}

void ConfigManager::save(const UserRouteMap& userRoutes) {
    try {
        nlohmann::json json;
        nlohmann::json usersJson = nlohmann::json::object();

        for (const auto& [userId, routes] : userRoutes) {
            nlohmann::json routesJson = nlohmann::json::object();
            
            for (const auto& [path, target] : routes) {
                routesJson[path] = target;
            }
            
            nlohmann::json userJson = nlohmann::json::object();
            userJson["routes"] = routesJson;
            usersJson[userId] = userJson;
        }

        json["users"] = usersJson;
        
        std::ofstream file(config_file);
        file << json.dump(4);
        spdlog::info("Configuration saved to file with {} users", userRoutes.size());
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config file: {}", e.what());
    }
}

void ConfigManager::updateUserRoutes(const std::string& userId, const RouteMap& routes) {
    // Load current config
    UserRouteMap allRoutes;
    load(allRoutes);
    
    // Update or add the specific user's routes
    allRoutes[userId] = routes;
    
    // Save all routes back
    save(allRoutes);
    
    spdlog::info("Updated routes for user {} in config file", userId);
}