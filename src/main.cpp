#include "server.h"

int main(){
    Server svr(8080);
    svr.start();
    return 0;
}