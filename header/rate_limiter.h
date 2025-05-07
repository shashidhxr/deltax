#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

class RateLimiter {
public:
    RateLimiter(size_t max_requests = 10, size_t window_seconds = 60);
    bool allow_req(const std::string& key);

private:
    struct Entry {
        size_t count;
        std::chrono::steady_clock::time_point reset_time;
    };

    std::unordered_map<std::string, Entry> clients_;
    std::mutex mutex_;
    size_t max_requests_;
    size_t window_seconds_;
};
