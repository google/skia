MSAN, ASAN, & TSAN
==================

*Testing Skia with memory, address, and thread santizers.*

Downloading Clang Binaries (Googlers Only)
------------------------------------------

    CLANGDIR="${HOME}/clang"
    python infra/bots/assets/clang_linux/download.py -t $CLANGDIR

Building Clang from scratch
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

You may also need to install the following packages to run the MSAN binary

    sudo apt-get install libc++dev libc++abi-dev


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


