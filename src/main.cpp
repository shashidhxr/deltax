#include "server.h"

int main() {
    Server server(9111, "/app/config.json");
    server.start();
    return 0;
}