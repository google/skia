FROM debian:9.4

# Enabling backports gives us access to clang-6 and new versions of libdrm package.
# Mesa builds newer than 18 or so require libdrm > 2.4.75, but the
# default one in stretch is 2.4.74
RUN echo "deb http://ftp.debian.org/debian stretch-backports main" >> /etc/apt/sources.list

RUN apt-get update && apt-get upgrade -y

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get install -y \
    autoconf \
    bison \
    build-essential \
    clang-6.0 \
    flex \
    libdrm-amdgpu1=2.4.91-2~bpo9+1 \
    libdrm-dev=2.4.91-2~bpo9+1 \
    libdrm-intel1=2.4.91-2~bpo9+1 \
    libdrm-nouveau2=2.4.91-2~bpo9+1 \
    libdrm-radeon1=2.4.91-2~bpo9+1 \
    libdrm2=2.4.91-2~bpo9+1 \
    libomxil-bellagio-dev \
    libpthread-stubs0-dev \
    libtool \
    libva-dev \
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

ENV CC="/usr/lib/llvm-6.0/bin/clang" \
    CXX="/usr/lib/llvm-6.0/bin/clang++" \
    PATH=/usr/lib/llvm-6.0/bin:$PATH

# Default to this version of MESA, but it can be overridden with
# -e MESA_VERSION=X.Y.Z when running the docker container
ENV MESA_VERSION=18.1.7

COPY ./build_mesa.sh /opt/build_mesa.sh