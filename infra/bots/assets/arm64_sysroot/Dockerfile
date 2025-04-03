# This was ubuntu:jammy on Jan 8 2025.
# Found by running
# docker pull ubuntu:jammy && docker images --digests | grep ubuntu
FROM ubuntu@sha256:ed1544e454989078f5dec1bfdabd8c5cc9c48e0705d07b678ab6ae3fb61952d2

# The versions were added after seeing what was available when this image was created on Jan 8 2025.
# Specifying the versions makes this Docker container comply with SLSA level 1.
RUN apt-get update && \
    apt-get install -y --no-install-recommends  \
    libstdc++-12-dev-arm64-cross=12.3.0-1ubuntu1~22.04cross1 \
    libgcc-12-dev-arm64-cross=12.3.0-1ubuntu1~22.04cross1 \
    binutils-arm-linux-gnueabihf=2.38-4ubuntu2.7 \
    && rm -rf /var/lib/apt/lists/*

ENV TARGET_DIR=/tmp/arm64_sysroot_output
RUN cp -RL /usr/aarch64-linux-gnu ${TARGET_DIR} && \
    cp -RL /usr/lib/gcc-cross/aarch64-linux-gnu/12 ${TARGET_DIR}/gcc-cross && \
    cp /usr/lib/x86_64-linux-gnu/libbfd-2.38-armhf.so ${TARGET_DIR}/lib && \
    cp /usr/lib/x86_64-linux-gnu/libopcodes-2.38-armhf.so ${TARGET_DIR}/lib && \
    cp /usr/lib/x86_64-linux-gnu/libctf-armhf.so.0.0.0 ${TARGET_DIR}/lib
RUN sed -i 's/\/usr\/aarch64-linux-gnu\/lib\///g' ${TARGET_DIR}/lib/libc.so && \
    sed -i 's/\/usr\/aarch64-linux-gnu\/lib\///g' ${TARGET_DIR}/lib/libm.so
