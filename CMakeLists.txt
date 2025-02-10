cmake_minimum_required(VERSION 3.16)
project(deltax)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(header)

find_package(httplib CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(deltax src/main.cpp src/server.cpp)

target_link_libraries(deltax PRIVATE spdlog::spdlog)
target_link_libraries(deltax PRIVATE httplib::httplib)
target_link_libraries(deltax PRIVATE nlohmann_json::nlohmann_json)
