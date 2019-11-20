FROM ubuntu:18.04

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  libfontconfig-dev \
  libglu-dev \
  python \
  && rm -rf /var/lib/apt/lists/*
