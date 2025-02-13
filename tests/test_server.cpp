#include <catch2/catch.hpp>
#include "server.h"

TEST_CASE("Server loads port from config file", "[server]") {
    int port = load_config();
    REQUIRE(port == 9090);  // Assuming 9000 is the port in `config.json`
}

TEST_CASE("Server responds with welcome message for root endpoint", "[server]") {
    Server server(8080);
    httplib::Client client("http://localhost:8080");

    auto res = client.Get("/");
    REQUIRE(res != nullptr);
    REQUIRE(res->status == 200);
    REQUIRE(res->body == "Welcome to deltax: API-Gateway");
}

TEST_CASE("Health endpoint responds with API stats", "[server]") {
    Server server(8080);
    httplib::Client client("http://localhost:8080");

    auto res = client.Get("/health");
    REQUIRE(res != nullptr);
    REQUIRE(res->status == 200);
    REQUIRE(res->body == "API stats");
}
