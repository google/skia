# Dockerfile for building Skia in release mode, using CMake.
FROM launcher.gcr.io/google/debian10

RUN echo "deb http://deb.debian.org/debian buster-backports main" >> /etc/apt/sources.list

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  python3 \
  curl \
  clang-11 \
  build-essential \
  cmake \
  libfreetype6-dev \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libxi-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 --home /workspace/__cache skia

RUN ln -s /usr/bin/clang-11 /usr/local/bin/clang && \
  ln -s /usr/bin/clang++-11 /usr/local/bin/clang++

RUN cd /opt \
 && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH="/opt/depot_tools:${PATH}"
