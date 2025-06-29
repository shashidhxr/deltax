#include "server.h"

int main() {
    Server server(9111);
    server.start();
    return 0;
}