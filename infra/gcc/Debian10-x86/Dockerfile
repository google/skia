FROM debian:10-slim

RUN dpkg --add-architecture i386 && \
  apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  g++-multilib \
  libfontconfig-dev:i386 \
  libglu-dev:i386 \
  python \
  && rm -rf /var/lib/apt/lists/*
