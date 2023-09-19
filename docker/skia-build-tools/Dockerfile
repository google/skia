# Dockerfile for building an image with all the tools needed to checkout and build Skia.
FROM launcher.gcr.io/google/debian11 AS build
RUN apt update && apt dist-upgrade -y && apt install -y \
  git \
  clang \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  procps \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 skia

# TODO(kjlubick): Try a shallow clone of depot_tools
RUN cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'

ENV PATH=${PATH}:/tmp/depot_tools

ADD --chown=skia:skia https://storage.googleapis.com/skia-swiftshader/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD --chown=skia:skia  https://storage.googleapis.com/skia-swiftshader/libEGL.so /usr/local/lib/libEGL.so