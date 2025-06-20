cmake_minimum_required(VERSION 3.16)
project(deltax)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

include_directories(header)
# add_subdirectory(nlohmann_json)
include_directories("/app/vcpkg/installed/x64-linux/include")

# find_package(httplib CONFIG REQUIRED)
# find_package(spdlog CONFIG REQUIRED)
# find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
# find_package(laserpants_dotenv REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(ZLIB REQUIRED)


# add_executable(deltax src/main.cpp src/server.cpp src/auth.cpp src/config.cpp)
add_executable( deltax 
                src/main.cpp 
                src/server.cpp 
                src/config.cpp 
                src/router 
                src/ws_client 
                src/rate_limiter.cpp)
# add_executable(main src/main.cpp)

# link_directories("/app/vcpkg/installed/x64-linux/lib")

# target_link_libraries(deltax PRIVATE httplib::httplib)
# target_link_libraries(deltax PRIVATE spdlog::spdlog)
# target_link_libraries(deltax PRIVATE nlohmann_json::nlohmann_json)
# target_link_libraries(deltax PRIVATE OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(main laserpants::dotenv)
# target_link_libraries(deltax PRIVATE ixwebsocket OpenSSL::SSL OpenSSL::Crypto)      # link issues 

target_link_libraries(deltax PRIVATE
    /app/vcpkg/installed/x64-linux/lib/libixwebsocket.a
    OpenSSL::SSL OpenSSL::Crypto
)
target_link_libraries(deltax PRIVATE fmt::fmt)

target_link_libraries(deltax PRIVATE ZLIB::ZLIB)

# test config

# find_package(Catch2 REQUIRED)

# add_executable(deltax_tests tests/test_auth.cpp, tests/test_server.cpp)

# target_link_libraries(deltax_tests PRIVATE spdlog::spdlog)
# target_link_libraries(deltax_tests PRIVATE httplib::httplib)
# target_link_libraries(deltax_tests PRIVATE nlohmann_json::nlohmann_json)
# target_link_libraries(deltax_tests PRIVATE OpenSSL::SSL OpenSSL::Crypto)
# target_link_libraries(deltax_tests PRIVATE Catch2::Catch2)

# enable_testing()
# add_test(NAME deltax_tests COMMAND deltax_tests)