# This is based off the official LLVM docker container
# https://github.com/llvm/llvm-project/blob/76fd4bf675b5ceeeca0e4e15cf15d89c7acf4947/llvm/utils/docker/debian10/Dockerfile
#
# This was launcher.gcr.io/google/debian10:latest on Sep 22 2022
# Found by running
# docker pull launcher.gcr.io/google/debian10:latest && docker images --digests | grep debian10
FROM launcher.gcr.io/google/debian10@sha256:3242ff21417c7722482c2085f86f28ed4f76cde00bf880f15fc1795975bc2a81

# Install build dependencies of llvm.
# First, Update the apt's source list and include the sources of the packages.
RUN grep deb /etc/apt/sources.list | \
    sed 's/^deb/deb-src /g' >> /etc/apt/sources.list
# Install compiler, python, etc. We need clang and lld because otherwise we have issues compiling
# compiler-rt (it fails using the built-in ld).
#
# The versions were added after seeing what was available when this image was created on Sep 22 2022
# Specifying the versions makes this Docker container comply with SLSA level 1.
RUN apt-get update && \
    apt-get install -y --no-install-recommends  \
           ca-certificates=20200601~deb10u2 gnupg=2.2.12-1+deb10u2 \
           build-essential=12.6 cmake=3.13.4-1 make=4.2.1-1.2 python3=3.7.3-1 \
           zlib1g=1:1.2.11.dfsg-1+deb10u2 wget=1.20.1-1.1 unzip=6.0-23+deb10u3 \
           git=1:2.20.1-2+deb10u3 clang=1:7.0-47 lld=1:7.0-47 && \
    rm -rf /var/lib/apt/lists/*
# Install a newer ninja release. It seems the older version in the debian repos
# randomly crashes when compiling llvm.
RUN wget "https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip" && \
    echo "d2fea9ff33b3ef353161ed906f260d565ca55b8ca0568fa07b1d2cab90a84a07 ninja-linux.zip" \
        | sha256sum -c  && \
    unzip ninja-linux.zip -d /usr/local/bin && \
    rm ninja-linux.zip

ENV TARGET_DIR=/tmp/clang_output
ENV CLANG_RELEASE=llvmorg-15.0.1

RUN mkdir -p /tmp/clang && cd /tmp/clang && \
    git clone --depth 1 -b ${CLANG_RELEASE} https://llvm.googlesource.com/llvm-project

# As of October 21, 2022, iwyu 0.19 (for clang 15) was not yet a branch, so we just checkout
# tip of tree at that time
RUN git clone https://github.com/include-what-you-use/include-what-you-use.git /tmp/iwyu && \
    cd /tmp/iwyu && \
    git checkout 6d416d5f90755dd76d55c0f33f9ac0de6dd27b5f

WORKDIR /tmp/clang/llvm-project

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

# https://libcxx.llvm.org/BuildingLibcxx.html#bootstrapping-build
# https://github.com/include-what-you-use/include-what-you-use#how-to-build-as-part-of-llvm
# This will build clang first and then use that new clang to build the runtimes and the
# iwyu probject.
RUN mkdir ${TARGET_DIR} out && \
    cmake -G Ninja -S llvm -B out \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${TARGET_DIR} \
    -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
    -DLLVM_ENABLE_RUNTIMES="all" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DLLVM_USE_LINKER=lld \
    -DLLVM_ENABLE_UNWIND_TABLES=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_EXTERNAL_PROJECTS=iwyu \
    -DLLVM_EXTERNAL_IWYU_SOURCE_DIR=/tmp/iwyu

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
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
    -DLLVM_USE_SANITIZER=Thread

RUN ninja -C tsan_out cxx cxxabi
RUN cp -r tsan_out/lib ${TARGET_DIR}/tsan

# We would be following the instructions from
# https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
# but those are currently out of date (https://github.com/google/sanitizers/issues/1574)
RUN mkdir msan_out && \
    cmake -GNinja -S runtimes -B msan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins

RUN ninja -C msan_out cxx cxxabi

RUN cp -r msan_out/lib ${TARGET_DIR}/msan