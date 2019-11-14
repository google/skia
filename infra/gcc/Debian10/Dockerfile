FROM debian:10-slim

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  python \
  && rm -rf /var/lib/apt/lists/*
