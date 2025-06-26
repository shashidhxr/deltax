# DeltaX

API gateway for workload-heavy tasks. Written in C++.


### Instructions to run locally

### 1. Build the Docker Image

```bash
docker build -t deltax:latest .

mkdir -p $PWD/deltax-config
cp ./config-volume/config.json $PWD/deltax-config/

docker run -d \
  -v $PWD/deltax-config:/app/config \
  -p 9111:9111 \
  --name deltax-runtime \
  deltax:latest

curl http://localhost:9111


### Features

- Reverse Proxy 
- Rate limiting  
- Load balancing  
- Auth
- API Keys Wallet

### Stack

- C++ core (actual gateway)
- Node.js (Express)
- PostgreSQL
- React frontend
- WebSockets for cross module comm
- REST APIs

Still in development. Bugs expected. Breaking things to learn and improve.

backend and db repo - https://github.com/shashidhxr/backend-deltax

frontend repo - https://github.com/shashidhxr/web-deltax

### Live

[https://deltax0.vercel.app](https://deltax0.vercel.app)
