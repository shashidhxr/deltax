#include "server.h"
#include <spdlog/spdlog.h>

int main() {
    // Configure logger
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    
    int port = 8111;
    
    spdlog::info("Starting API Gateway on port {}", port);
    Server server(port);
    server.start();
    return 0;
}