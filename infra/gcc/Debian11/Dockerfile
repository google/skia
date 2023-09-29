FROM debian:11-slim

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  ca-certificates \
  libfontconfig-dev \
  libglu-dev \
  python3 \
  wget \
  unzip \
  lld \
  git \
  && rm -rf /var/lib/apt/lists/*
RUN ln -s /usr/bin/python3 /usr/bin/python
