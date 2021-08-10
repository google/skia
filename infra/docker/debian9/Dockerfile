FROM debian:9-slim

# See https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=863199
# Needed for installing openjdk-8-headless on debian:9-slim
RUN mkdir -p /usr/share/man/man1

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  mercurial \
  libosmesa-dev \
  libexpat1-dev \
  clang \
  llvm \
  poppler-utils \
  netpbm \
  gcc-multilib \
  g++-multilib \
  openjdk-8-jdk-headless \
  libxi-dev \
  python3-django \
  libc++-dev \
  libc++abi-dev \
  gperf \
  bison \
  usbutils \
  libfontconfig-dev \
  libglu-dev \
  python3 \
  git \
  && rm -rf /var/lib/apt/lists/*
