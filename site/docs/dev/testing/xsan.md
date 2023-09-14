
---
title: "MSAN, ASAN, & TSAN"
linkTitle: "MSAN, ASAN, & TSAN"

---


*Testing Skia with memory, address, and thread santizers.*

Compiling Skia with ASAN, UBSAN, or TSAN can be done with the latest version of Clang.

- UBSAN works on Linux, Mac, Android, and Windows, though some checks are platform-specific.
- ASAN works on Linux, Mac, Android, and Windows.
- TSAN works on Linux and Mac.
- MSAN works on Linux[1].

We find that testing sanitizer builds with libc++ uncovers more issues than
with the system-provided C++ standard library, which is usually libstdc++.
libc++ proactively hooks into sanitizers to help their analyses.
We ship a copy of libc++ with our Linux toolchain in /lib.

[1]To compile and run with MSAN, an MSAN-instrumented version of libc++ is needed.
It's generally easiest to run one of the following 2 steps to build/download a recent version
of Clang and the instrumented libc++, located in /msan.

Downloading Clang binaries (Googlers Only)
------------------------------------------
This requires gsutil, part of the [gcloud sdk](https://cloud.google.com/sdk/downloads).

<!--?prettify lang=sh?-->

    gcloud auth application-default login
    CLANGDIR="${HOME}/clang"
    ./bin/sk asset download clang_linux $CLANGDIR

Building Clang binaries from scratch (Other users)
---------------------------

<!--?prettify lang=sh?-->

    CLANGDIR="${HOME}/clang"

    python3 tools/git-sync-deps
    CC= CXX= infra/bots/assets/clang_linux/create.py -t "$CLANGDIR"

Configure and Compile Skia with MSAN
------------------------------------

<!--?prettify lang=sh?-->

    CLANGDIR="${HOME}/clang"
    mkdir -p out/msan
    cat > out/msan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        extra_cflags = [ "-B${CLANGDIR}/bin" ]
        extra_ldflags = [
            "-B${CLANGDIR}/bin",
            "-fuse-ld=lld",
            "-L${CLANGDIR}/msan",
            "-Wl,-rpath,${CLANGDIR}/msan" ]
        sanitize = "MSAN"
        skia_use_fontconfig = false
    EOF
    python3 tools/git-sync-deps
    bin/gn gen out/msan
    ninja -C out/msan

Configure and Compile Skia with ASAN
------------------------------------

<!--?prettify lang=sh?-->

    CLANGDIR="${HOME}/clang"
    mkdir -p out/asan
    cat > out/asan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "ASAN"
        extra_ldflags = [ "-fuse-ld=lld", "-Wl,-rpath,${CLANGDIR}/lib/x86_64-unknown-linux-gnu" ]
    EOF
    python3 tools/git-sync-deps
    bin/gn gen out/asan
    ninja -C out/asan

Configure and Compile Skia with TSAN
------------------------------------

<!--?prettify lang=sh?-->

    CLANGDIR="${HOME}/clang"
    mkdir -p out/tsan
    cat > out/tsan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "TSAN"
        is_debug = false
        extra_ldflags = [ "-Wl,-rpath,${CLANGDIR}/lib" ]
    EOF
    python3 tools/git-sync-deps
    bin/gn gen out/tsan
    ninja -C out/tsan


