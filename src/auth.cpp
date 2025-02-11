#include "auth.h"
#include <spdlog/spdlog.h>

bool validate_jwt(const auto &token){
    return true;
}

bool verify_token(const httplib::Request &req)
{
    auto auth_header = req.get_header_value("Authorization");
    if(auth_header.empty()){
        return false;
    }
    // to remove Bearer prefix
    std::string token = auth_header.substr(7);
    return validate_jwt(token);
}
