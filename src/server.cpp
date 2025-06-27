#include "server.h"

#include <spdlog/spdlog.h>

Server::Server(int port)
    : port(port),
      db(),
      configManager(db),
      router(db),
      wsClient(configManager) {
    configManager.loadInitConfig();  // rf1- replace this with cold start logic or maybe push from nodejs
}

void Server::start() {
    // wsClient.connect("ws://localhost:5000");
    wsClient.connect("wss://backend-deltax.onrender.com/ws"); 
    
    router.setupRouteHandler(svr);
    spdlog::info("API Gateway listening on port {}", port);
    svr.listen("0.0.0.0", port);
}
