#include "server.h"

int main(){
    int port = load_config();

    Server svr(port);
    svr.start();
    return 0;
}