// server/ConfigManager.h
#pragma once

#include <string>
#include <unordered_map>

class ConfigManager {
public:
    ConfigManager(const std::string& path);
    bool load(std::unordered_map<std::string, std::string>& routes);
    void save(const std::unordered_map<std::string, std::string>& routes);

private:
    std::string config_file;
};
