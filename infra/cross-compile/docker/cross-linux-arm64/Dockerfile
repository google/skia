FROM launcher.gcr.io/google/clang-debian9 AS build

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  binutils-aarch64-linux-gnu \
  git \
  libc6-dev-arm64-cross \
  libegl1-mesa-dev \
  libstdc++-6-dev-arm64-cross \
  python

RUN cd /opt \
 && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH="/opt/depot_tools:${PATH}"
