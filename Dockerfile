FROM gcc:latest

RUN apt update && apt install -y cmake openssl libssl-dev

WORKDIR /app

# COPY config.json .
# COPY config.json ./config.template.json
    
COPY src/ ./src
COPY header/ ./header
COPY CMakeLists.txt .

COPY vcpkg-headers/nlohmann ./vcpkg-headers/nlohmann
COPY vcpkg-headers /app/vcpkg/installed/x64-linux/include

# COPY docker-vcpkg/installed/x64-linux/include /app/vcpkg/installed/x64-linux/include
# COPY docker-vcpkg/installed/x64-linux/share /app/vcpkg/installed/x64-linux/share

COPY docker-vcpkg/installed/x64-linux/include /app/vcpkg/installed/x64-linux/include
COPY docker-vcpkg/installed/x64-linux/share /app/vcpkg/installed/x64-linux/share
COPY docker-vcpkg/installed/x64-linux/lib /app/vcpkg/installed/x64-linux/lib
COPY docker-vcpkg/installed/x64-linux/debug /app/vcpkg/installed/x64-linux/debug

RUN mkdir build && cd build && cmake .. -DCMAKE_PREFIX_PATH="/app/vcpkg/installed/x64-linux/share" && make

CMD ["./build/deltax"]
