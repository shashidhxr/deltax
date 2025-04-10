#include "server.h"

int main() {
    Server server(9111, "../config.json");
    server.start();
    return 0;
}