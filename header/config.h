// server/ConfigManager.h
#pragma once

#include <string>
#include <unordered_map>

using RouteMap = std::unordered_map<std::string, std::string>;
using UserRouteMap = std::unordered_map<std::string, RouteMap>;

class ConfigManager {
private:
    std::string config_file;

public:
    ConfigManager(const std::string& path);
    
    bool load(UserRouteMap& userRoutes);
    void save(const UserRouteMap& userRoutes);    
    void updateUserRoutes(const std::string& userId, const RouteMap& routes);
};