# Dockerfile for building Skia in release mode, using CMake.
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libxi-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 skia

RUN curl -s "https://cmake.org/files/v3.13/cmake-3.13.1-Linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local

RUN cd /opt \
 && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH="/opt/depot_tools:${PATH}"
