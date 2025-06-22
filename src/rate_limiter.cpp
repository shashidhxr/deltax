#include "rate_limiter.h"

RateLimiter::RateLimiter(size_t max_requests, size_t window_seconds)
    : max_requests_(max_requests), window_seconds_(window_seconds) {}

bool RateLimiter::allow_req(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::steady_clock::now();
    auto& entry = clients_[key];

    if (entry.reset_time <= now) {
        entry.count = 0;
        entry.reset_time = now + std::chrono::seconds(window_seconds_);
    }

    if (entry.count >= max_requests_) {
        return false;
    }

    ++entry.count;
    return true;
}
