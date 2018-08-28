FROM ubuntu:18.04

RUN apt-get update && apt-get upgrade -y

RUN apt-get install -y \
    autoconf \
    bison \
    build-essential \
    flex \
    libdrm-dev \
    libpthread-stubs0-dev \
    libtool \
    libx11-xcb-dev \
    libxcb-dri2-0-dev \
    libxcb-dri3-dev \
    libxcb-glx0-dev \
    libxcb-present-dev \
    libxcb1-dev \
    libxdamage-dev \
    libxext-dev \
    libxshmfence-dev \
    llvm-dev \
    pkg-config \
    python-pip \
    python2.7 \
    scons \
    software-properties-common \
    wget \
    x11proto-dri2-dev \
    x11proto-dri3-dev \
    x11proto-gl-dev \
    x11proto-present-dev \
    x11proto-xext-dev \
    xserver-xorg-core \
    xserver-xorg-dev

RUN pip install mako

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" \
    && apt-get update \
    && apt-get install -y clang-6.0

# Default to this version of MESA, but it can be overridden with
# -e MESA_VERSION=X.Y.Z when running the docker container
ENV MESA_VERSION=18.1.7

COPY ./build_mesa.sh /opt/build_mesa.sh