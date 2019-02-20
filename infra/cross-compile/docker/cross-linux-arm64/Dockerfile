FROM launcher.gcr.io/google/clang-debian9 AS build

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y \
  binutils-aarch64-linux-gnu \
  libc6-dev-arm64-cross \
  libegl1-mesa-dev \
  libstdc++-6-dev-arm64-cross \
  python
