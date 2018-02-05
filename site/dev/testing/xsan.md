MSAN, ASAN, & TSAN
==================

*Testing Skia with memory, address, and thread santizers.*

Compiling Skia with ASAN, UBSAN, or TSAN can be done with the latest version of Clang.

- UBSAN works on Linux, Mac, Android, and Windows, though some checks are platform-specific.
- ASAN works on Linux, Mac, Android.
- TSAN works on Linux and Mac.
- MSAN works on Linux[1].

[1]To compile and run with MSAN, an MSAN-instrumented version of libc++ is needed.
It's generally easiest to run one of the following 2 steps to build/download a recent version
of Clang and the instrumented libc++.

Downloading Clang binaries (Googlers Only)
------------------------------------------

    CLANGDIR="${HOME}/clang"
    python infra/bots/assets/clang_linux/download.py -t $CLANGDIR

Building Clang binaries from scratch (Other users)
---------------------------

    CLANGDIR="${HOME}/clang"

    python tools/git-sync-deps
    CC= CXX= infra/bots/assets/clang_linux/create.py -t "$CLANGDIR"

Configure and Compile Skia with MSAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/msan
    cat > out/msan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        extra_cflags = [ "-B${CLANGDIR}/bin" ]
        extra_ldflags = [ "-B${CLANGDIR}/bin", "-fuse-ld=lld", "-L${CLANGDIR}/msan" ]
        sanitize = "MSAN"
        skia_use_fontconfig = false
    EOF
    python tools/git-sync-deps
    bin/gn gen out/msan
    ninja -C out/msan

When you run a binary built with MSAN, make sure you force it to use our
MSAN-instrumented libc++:

    env LD_LIBRARY_PATH=$CLANGDIR/msan out/dm ...

Configure and Compile Skia with ASAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/asan
    cat > out/asan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "ASAN"
    EOF
    python tools/git-sync-deps
    bin/gn gen out/asan
    ninja -C out/asan

Configure and Compile Skia with TSAN
------------------------------------

    CLANGDIR="${HOME}/clang"
    mkdir -p out/tsan
    cat > out/tsan/args.gn <<- EOF
        cc = "${CLANGDIR}/bin/clang"
        cxx = "${CLANGDIR}/bin/clang++"
        sanitize = "TSAN"
        is_debug = false
    EOF
    python tools/git-sync-deps
    bin/gn gen out/tsan
    ninja -C out/tsan


