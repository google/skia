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
           build-essential=12.6 make=4.2.1-1.2 python3=3.7.3-1 \
           zlib1g=1:1.2.11.dfsg-1+deb10u2 wget=1.20.1-1.1 unzip=6.0-23+deb10u3 \
           git=1:2.20.1-2+deb10u3 clang=1:7.0-47 lld=1:7.0-47 && \
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
ENV CLANG_RELEASE=llvmorg-18.1.8

RUN mkdir -p /tmp/clang && cd /tmp/clang && \
    git clone --depth 1 -b ${CLANG_RELEASE} https://llvm.googlesource.com/llvm-project

# As of August 30, 2024, iwyu 0.22 (for clang 18) is a branch. Unfortunately, this branch requires
# python 3.9. Commit "Loosen Python version requirements for functools.cache" [0] restored support
# for python 3.2+. However, clang internally made an incompatable change, which iwyu picked up with
# "[clang compat] Update InclusionDirective override" [1] which came before it. So we must
# checkout 0.22 and cherry-pick the python fix.
# [0] https://github.com/include-what-you-use/include-what-you-use/commit/88e414728702390ed8eebf11d14b992a01df1ee0
# [1] https://github.com/include-what-you-use/include-what-you-use/commit/f27fafa378a70b975aeaa9a1428b275a1edcfe5a
RUN git clone https://github.com/include-what-you-use/include-what-you-use.git /tmp/iwyu && \
    cd /tmp/iwyu && \
    git checkout 377eaef70cdda47368939f4d9beabfabe3f628f0 && \
    git config user.email "sk_asset_create@example.com" && \
    git config user.name "SkAsset Create" && \
    git cherry-pick 88e414728702390ed8eebf11d14b992a01df1ee0

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
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
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
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=Thread

RUN ninja -C tsan_out cxx cxxabi
RUN cp -r tsan_out/lib ${TARGET_DIR}/tsan

# We would be following the instructions from
# https://github.com/google/sanitizers/wiki/MemorySanitizerLibcxxHowTo
# but those are currently out of date (https://github.com/google/sanitizers/issues/1574)
RUN mkdir msan_out && \
    cmake -GNinja -S runtimes -B msan_out \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind;compiler-rt" \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins

RUN ninja -C msan_out cxx cxxabi

RUN cp -r msan_out/lib ${TARGET_DIR}/msan
