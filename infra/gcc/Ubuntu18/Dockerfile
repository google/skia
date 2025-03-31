# This was ubuntu:bionic on Mar 27 2025.
FROM ubuntu@sha256:152dc042452c496007f07ca9127571cb9c29697f42acbfad72324b2bb2e43c98

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  gcc-4.8 \
  libfontconfig-dev \
  libglu-dev \
  libstdc++-8-dev \
  lld \
  python3 \
  wget \
  unzip \
  lld \
  git \
  && rm -rf /var/lib/apt/lists/*
