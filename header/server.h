#ifndef SERVER_H
#define SERVER_H

#include "httplib.h"

class Server {
public:
    Server(int port);
    void start();
private:
    httplib::Server svr;
    int port;        
};

#endif