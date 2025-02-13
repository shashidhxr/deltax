#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <httplib.h>

bool validate_jwt(const std::string &token);
bool verify_token(const httplib::Request &req);

#endif 
