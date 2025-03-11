#include "server.h"
#include "auth.h"
// #include <dotenv.h>

int main(){
    // int port = load_config();
    int port = 8111;

    Server svr(port);
    svr.start();
    return 0;
}