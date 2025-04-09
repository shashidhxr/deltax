#include "server.h"

#include <spdlog/spdlog.h>

Server::Server(int port, const std::string& config_path)
    : port(port), configManager(config_path), wsClient(router, configManager) {
    configManager.load(router.getRoutesMutable());
}

void Server::start() {
    wsClient.connect("ws://localhost:5000");
    router.setupRouteHandler(svr);
    spdlog::info("API Gateway listening on port {}", port);
    svr.listen("0.0.0.0", port);
}
