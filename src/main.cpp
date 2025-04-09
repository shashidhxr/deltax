#include "server.h"

int main() {
    Server server(3005, "config.json");
    server.start();
    return 0;
}