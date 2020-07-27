Reproducing Skia Fuzzes
=======================

We assume that you can [build Skia](/user/build). Many fuzzes only reproduce
when building with ASAN or MSAN; see [those instructions for more details](./xsan).

When building, you should add the following args to BUILD.gn to make reproducing
less machine- and platform- dependent:

    skia_use_fontconfig=false
    skia_use_freetype=true
    skia_use_system_freetype2=false
    skia_use_wuffs=true
    skia_enable_skottie=true
    skia_enable_fontmgr_custom_directory=false
    skia_enable_fontmgr_custom_embedded=false
    skia_enable_fontmgr_custom_empty=true

All that is needed to reproduce a fuzz downloaded from ClusterFuzz, oss-fuzz or
fuzzer.skia.org is to run something like:

    out/ASAN/fuzz -b /path/to/downloaded/testcase

The fuzz binary will try its best to guess what the type/name should be based on
the name of the testcase. Manually providing type and name is also supported, like:

    out/ASAN/fuzz -t filter_fuzz -b /path/to/downloaded/testcase
    out/ASAN/fuzz -t api -n RasterN32Canvas -b /path/to/downloaded/testcase

To enumerate all supported types and names, run the following:

    out/ASAN/fuzz --help  # will list all types
    out/ASAN/fuzz -t api  # will list all names

If the crash does not show up, try to add the flag --loops:

    out/ASAN/fuzz -b /path/to/downloaded/testcase --loops <times-to-run>

To backend a fuzzer with swiftshader and run locally, download the files and source code:

    wget https://storage.googleapis.com/skia-swiftshader/libGLESv2.so -P /opt/swiftshader/lib
    wget https://storage.googleapis.com/skia-swiftshader/libEGL.so -P /opt/swiftshader/lib
    git clone https://swiftshader.googlesource.com/SwiftShader /opt/swiftshader/src

Then create folder and configuration files:

    mkdir -p skia/out/with-swift-shader
    echo 'cc = "clang"
    cxx = "clang++"
    skia_use_egl = true
    is_debug = false
    skia_use_system_freetype2 = false
    extra_cflags = [
        "-I/opt/swiftshader/src/include",
        "-DGR_EGL_TRY_GLES3_THEN_GLES2",
    ]
    extra_ldflags = [
        "-L/opt/swiftshader/lib"
    ] ' > ./out/with-swift-shader/args.gn

Build swiftshader and compile with ninja:

    gn args skia/out/with-swift-shader/
    ninja -C ./out/with-swift-shader fuzz

Run fuzzer with swiftshader:

    skia/out/with-swift-shader/fuzz -t api -n RasterN32Canvas
