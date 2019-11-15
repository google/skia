FROM debian:10-slim

RUN dpkg --add-architecture mips64el && \
  apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  g++-multilib-mips64el-linux-gnuabi64 \
  libfontconfig-dev:mips64el \
  libglu-dev:mips64el \
  python \
  && rm -rf /var/lib/apt/lists/*
