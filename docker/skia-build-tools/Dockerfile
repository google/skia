# Dockerfile for building an image with all the tools needed to checkout and build Skia.
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  wget \
  python \
  python3 \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 skia

# TODO(kjlubick): Try a shallow clone of depot_tools
RUN cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH=${PATH}:/tmp/depot_tools

RUN cd /tmp \
  && wget --quiet https://github.com/Kitware/CMake/releases/download/v3.18.0/cmake-3.18.0-Linux-x86_64.sh \
  && chmod +x cmake-3.18.0-Linux-x86_64.sh

RUN cd /tmp \
  && ./cmake-3.18.0-Linux-x86_64.sh --skip-licenses --prefix=/; exit 0

RUN which cmake \
  && cmake --version

# Should this use the Skia deps of SwiftShader?
RUN cd /tmp \
  && git clone https://swiftshader.googlesource.com/SwiftShader \
  && cd SwiftShader \
    && git checkout 97f9923235cf5a24d8ce49f93bd22429d2a68548 \
    && cd build \
  && cmake .. \
  && make --jobs=8

#ADD --chown=skia:skia https://storage.googleapis.com/skia-swiftshader/libGLESv2.so /usr/local/lib/libGLESv2.so
#ADD --chown=skia:skia  https://storage.googleapis.com/skia-swiftshader/libEGL.so /usr/local/lib/libEGL.so