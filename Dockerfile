FROM ubuntu:22.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    openssl \
    libssl-dev \
    git

WORKDIR /app

COPY src/ ./src
COPY header/ ./header
COPY CmakeLists.txt .

RUN mkdir build && cd build && cmake .. && make

CMD ["./build/deltax"]