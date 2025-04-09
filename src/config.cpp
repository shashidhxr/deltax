// server/ConfigManager.cpp
#include "config.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

ConfigManager::ConfigManager(const std::string& path) : config_file(path) {}

bool ConfigManager::load(std::unordered_map<std::string, std::string>& routes) {
    try {
        std::ifstream file(config_file);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;

            routes.clear();
            for (auto& [key, value] : json["routes"].items()) {
                routes[key] = value;
            }
            spdlog::info("Loaded configuration from file with {} routes", routes.size());
            return true;
        }
        spdlog::warn("Config file not found, starting with empty configuration");
        return false;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config file: {}", e.what());
        return false;
    }
}

void ConfigManager::save(const std::unordered_map<std::string, std::string>& routes) {
    try {
        nlohmann::json json;
        nlohmann::json route_json;

        for (const auto& [path, target] : routes) {
            route_json[path] = target;
        }

        json["routes"] = route_json;
        std::ofstream file(config_file);
        file << json.dump(4);
        spdlog::info("Configuration saved to file");
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config file: {}", e.what());
    }
}