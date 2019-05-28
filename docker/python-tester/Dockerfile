# Dockerfile for testing Python programs with both v2 and v3
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update \
  && apt-get upgrade -y \
  && apt-get install -y \
     python \
     python3 \
     git \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 skia

USER skia
