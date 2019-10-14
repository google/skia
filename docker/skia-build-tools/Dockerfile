# Dockerfile for building an image with all the tools needed to build Skia.
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 skia

ADD --chown=skia:skia https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD --chown=skia:skia  https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libEGL.so /usr/local/lib/libEGL.so