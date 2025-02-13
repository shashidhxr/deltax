#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "auth.h"

// Mock token for testing
std::string valid_token = jwt::create()
    .set_issuer("auth0")
    .set_type("JWT")
    .set_payload_claim("user_id", jwt::claim(std::string("12345")))
    .sign(jwt::algorithm::hs256{"secret-key"});

std::string invalid_token = "invalid.token.value";

TEST_CASE("validate_jwt returns true for valid JWT", "[auth]") {
    REQUIRE(validate_jwt(valid_token) == true);
}

TEST_CASE("validate_jwt returns false for invalid JWT", "[auth]") {
    REQUIRE(validate_jwt(invalid_token) == false);
}

TEST_CASE("verify_token returns false for empty Authorization header", "[auth]") {
    httplib::Request req;
    req.set_header("Authorization", "");
    REQUIRE(verify_token(req) == false);
}

TEST_CASE("verify_token returns true for valid Authorization header", "[auth]") {
    httplib::Request req;
    req.set_header("Authorization", "Bearer " + valid_token);
    REQUIRE(verify_token(req) == true);
}
