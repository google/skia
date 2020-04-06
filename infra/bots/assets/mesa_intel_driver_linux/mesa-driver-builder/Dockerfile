FROM debian:10.3

ENV DEBIAN_FRONTEND noninteractive

# Enabling backports gives us access to clang-6 and makes it more likely that
# 'apt-get build-dep' will install the correct dependencies.
# Mesa builds newer than 17.0.4 or so require libdrm > 2.4.75, but the
# default one in stretch is 2.4.74.
# Note that the hosts that use these drivers will also need the newer version of libdrm2
# or there will be an error along the lines of:
#     symbol lookup error: ./mesa_intel_driver/libGL.so.1: undefined symbol: drmGetDevice2
#
# Hosts can install this by adding the stretch-backports debian source (see next RUN)
# and then performing `sudo apt-get update && sudo apt-get install libdrm2=2.4.95-1~bpo9+1`
RUN echo "deb-src http://deb.debian.org/debian buster main" >> /etc/apt/sources.list && \
    echo "deb-src http://deb.debian.org/debian buster-updates main" >> /etc/apt/sources.list && \
    apt-get update && apt-get upgrade -y && \
    apt-get install -y wget clang-6.0 && \
    apt-get build-dep -y mesa && \
    rm -rf /var/lib/apt/lists/*

ENV CC="/usr/lib/llvm-6.0/bin/clang" \
    CXX="/usr/lib/llvm-6.0/bin/clang++" \
    PATH=/usr/lib/llvm-6.0/bin:$PATH \
    # Default to this version of MESA, but it can be overridden with
    # -e MESA_VERSION=X.Y.Z when running the docker container
    MESA_VERSION=18.3.6

COPY ./build_mesa.sh /opt/build_mesa.sh
