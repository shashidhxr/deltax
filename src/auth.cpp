#include "auth.h"
#include <spdlog/spdlog.h>
#include <jwt-cpp/jwt.h>

// todo - load from env var
std::string secret_key = "secret-key";

bool validate_jwt(const std::string &token){
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{secret_key})
                                .with_issuer("auth0");
        verifier.verify(decoded);
        spdlog::info("JWT valdiation successful");
        return true;
    } catch(const std::exception &e){
        spdlog::error("JWT validation failed"); 
        return false;        
    }
}

bool verify_token(const httplib::Request &req)
{
    auto auth_header = req.get_header_value("Authorization");
    if(auth_header.empty()){
        return false;
    }
    // std::cout << auth_header << std::endl;
    // to remove Bearer prefix
    std::string token = auth_header.substr(7);
    // std::cout << token << std::endl;
    // std::string token = auth_header;
    return validate_jwt(token);
}
