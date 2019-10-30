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
    skia_enable_fontmgr_custom=false
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
