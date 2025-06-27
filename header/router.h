// server/Router.h
#pragma once

#include "im_db.h"

#include <string>
#include <httplib.h>

// #include "../vcpkg-headers/httplib.h"
// #include "../vcpkg-headers/nlohmann/json.hpp"

class Router {
private:
    InMemoryDB& dbRef;          // ref to existing db object

public:
    Router(InMemoryDB& db);         // dependency injection, no state duplication

    void setupRouteHandler(httplib::Server& svr);
};