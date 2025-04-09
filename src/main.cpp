#include "server.hpp"
#include <spdlog/spdlog.h>

int main() {
    // Configure logger with more detailed output
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    
    int port = 8111;
    
    spdlog::info("Starting API Gateway on port {}", port);
    Server server(port);
    server.start();
    return 0;
}