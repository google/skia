FROM debian:10-slim

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  libfontconfig-dev \
  libglu-dev \
  python \
  && rm -rf /var/lib/apt/lists/*
