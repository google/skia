# This is based off the official LLVM docker container
# https://github.com/llvm/llvm-project/blob/76fd4bf675b5ceeeca0e4e15cf15d89c7acf4947/llvm/utils/docker/debian10/Dockerfile
#
# This was launcher.gcr.io/google/debian11:latest on May 28 2026
# Found by running
# docker pull launcher.gcr.io/google/debian11:latest && docker images --digests | grep debian11
FROM launcher.gcr.io/google/debian11@sha256:f456f8014492a6901ee6631e70c27742a2b11edaa4641dfcc382a05ae5f2f6f2

# Install build dependencies of llvm.
# First, Update the apt's source list and include the sources of the packages.
RUN grep deb /etc/apt/sources.list | \
    sed 's/^deb/deb-src /g' >> /etc/apt/sources.list
# Install compiler, python, etc. We need clang and lld because otherwise we have issues compiling
# compiler-rt (it fails using the built-in ld).
#
# The versions were added after seeing what was available when this image was created on May 28 2026
# by running:
# docker run --rm launcher.gcr.io/google/debian11@sha256:f456f8014492a6901ee6631e70c27742a2b11edaa4641dfcc382a05ae5f2f6f2 \
#    /bin/bash -c "apt-get update > /dev/null && apt-cache policy ca-certificates gnupg build-essential \
#    make python3 zlib1g wget unzip git clang lld"
#
# Specifying the versions makes this Docker container comply with SLSA level 1.
RUN apt-get update && \
    apt-get install -y --no-install-recommends  \
           ca-certificates=20230311+deb12u1~deb11u1 gnupg=2.2.27-2+deb11u3 \
           build-essential=12.9 make=4.3-4.1 python3=3.9.2-3 \
           zlib1g=1:1.2.11.dfsg-2+deb11u2 wget=1.21-1+deb11u2 unzip=6.0-26+deb11u1 \
           git=1:2.30.2-1+deb11u5 clang=1:11.0-51+nmu5 lld=1:11.0-51+nmu5 && \
    rm -rf /var/lib/apt/lists/*

# Install a newer ninja release.
RUN wget "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip" && \
    echo "6f98805688d19672bd699fbbfa2c2cf0fc054ac3df1f0e6a47664d963d530255 ninja-linux.zip" \
        | sha256sum -c  && \
    unzip ninja-linux.zip -d /usr/local/bin && \
    rm ninja-linux.zip

# Install a newer CMake release
RUN wget "https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3-linux-x86_64.tar.gz" && \
    echo "4a5864e9ff0d7945731fe6d14afb61490bf0ec154527bc3af0456bd8fa90decb cmake-3.30.3-linux-x86_64.tar.gz" \
        | sha256sum -c && \
    tar --strip-components=1 -xvzf cmake-3.30.3-linux-x86_64.tar.gz -C /usr/local && \
    rm cmake-3.30.3-linux-x86_64.tar.gz

ENV TARGET_DIR=/tmp/clang_output
ENV CLANG_RELEASE=llvmorg-22.1.0

RUN mkdir -p /tmp/clang && cd /tmp/clang && \
    git clone --depth 1 -b ${CLANG_RELEASE} https://llvm.googlesource.com/llvm-project

WORKDIR /tmp/clang/llvm-project

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

# https://libcxx.llvm.org/BuildingLibcxx.html#bootstrapping-build
# This will build clang first and then use that new clang to build the runtimes.
RUN mkdir ${TARGET_DIR} out && \
    cmake -G Ninja -S llvm -B out \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${TARGET_DIR} \
    -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DLLVM_USE_LINKER=lld \
    -DLLVM_ENABLE_UNWIND_TABLES=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF

RUN ninja -C out install
RUN cp out/bin/llvm-symbolizer out/bin/llvm-profdata out/bin/llvm-cov ${TARGET_DIR}/bin
RUN cp `c++ -print-file-name=libstdc++.so.6` ${TARGET_DIR}/lib

# Use the newly compiled clang to build TSAN and MSAN libraries.
ENV CC=${TARGET_DIR}/bin/clang
ENV CXX=${TARGET_DIR}/bin/clang++

# It is very important to start the build from the runtimes subfolder and not the llvm subfolder
# like we did above when following the bootstrapping-build instructions.
# https://stackoverflow.com/a/73827100/1447621
RUN mkdir tsan_out && \
    cmake -G Ninja -S runtimes -B tsan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=Thread

RUN ninja -C tsan_out cxx cxxabi
RUN cp -r tsan_out/lib ${TARGET_DIR}/tsan

# We would be following the instructions from
# https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
# but those are currently out of date (https://github.com/google/sanitizers/issues/1574)

# It is important that libunwind NOT be instrumented with MSAN, otherwise
# it can lead to infinite recursion during stack unwinding.
RUN mkdir msan_unwind_out && \
    cmake -G Ninja -S runtimes -B msan_unwind_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libunwind"

RUN ninja -C msan_unwind_out unwind

RUN mkdir msan_out && \
    cmake -GNinja -S runtimes -B msan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins

RUN ninja -C msan_out cxx cxxabi

RUN mkdir -p ${TARGET_DIR}/msan
RUN cp -r msan_out/lib/* ${TARGET_DIR}/msan
# Overwrite the instrumented libunwind with the uninstrumented one.
RUN cp -r msan_unwind_out/lib/libunwind* ${TARGET_DIR}/msan
