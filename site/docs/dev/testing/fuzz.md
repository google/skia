---
title: 'Fuzzing'
linkTitle: 'Fuzzing'
---

## Reproducing using `fuzz`

We assume that you can [build Skia](/docs/user/build). Many fuzzes only
reproduce when building with ASAN or MSAN; see
[those instructions for more details](../xsan).

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

All that is needed to reproduce a fuzz downloaded from ClusterFuzz or oss-fuzz
is to run something like:

    out/ASAN/fuzz -b /path/to/downloaded/testcase

The fuzz binary will try its best to guess what the type/name should be based on
the name of the testcase. Manually providing type and name is also supported,
like:

    out/ASAN/fuzz -t filter_fuzz -b /path/to/downloaded/testcase
    out/ASAN/fuzz -t api -n RasterN32Canvas -b /path/to/downloaded/testcase

To enumerate all supported types and names, run the following:

    out/ASAN/fuzz --help  # will list all types
    out/ASAN/fuzz -t api  # will list all names

If the crash does not show up, try to add the flag --loops:

    out/ASAN/fuzz -b /path/to/downloaded/testcase --loops <times-to-run>

## Writing fuzzers with libfuzzer

libfuzzer is an easy way to write new fuzzers, and how we run them on oss-fuzz.
Your fuzzer entry point should implement this API:

    extern "C" int LLVMFuzzerTestOneInput(const uint8_t*, size_t);

First install Clang (which should include libfuzzer):

    sudo apt install clang-19

Set up GN args to use libfuzzer:

    cc="clang-19"
    cxx="clang++-19"
    skia_build_fuzzers=true

Add an entry to BUILD.gn using `libfuzzer_app`

Build all the fuzz targets:

    ninja -C out/fuzz my_new_fuzzer

Run your new fuzzer binary to test it out

    out/fuzz/my_new_fuzzer

After adding a new fuzzer, one should connect it with //fuzz/FuzzMain.cpp and then update
**OSS-Fuzz** (see below) to get the fuzzer running automatically.

## Fuzzing Defines

There are some defines that can help guide a fuzzer to be more productive (e.g.
avoid OOMs, avoid unnecessarily slow code).

    // Required for fuzzing with afl-fuzz to prevent OOMs from adding noise.
    SK_BUILD_FOR_AFL_FUZZ

    // Required for fuzzing with libfuzzer
    SK_BUILD_FOR_LIBFUZZER

    // This define adds in guards to abort when we think some code path will take a long time or
    // use a lot of RAM. It is set by default when either of the above defines are set.
    SK_BUILD_FOR_FUZZER

## OSS-Fuzz
The infrastructure that our fuzzers run on is called [OSS-Fuzz](https://google.github.io/oss-fuzz/)
([GitHub](https://github.com/google/oss-fuzz/tree/master)). There is an automated system that
rebuilds Skia and certain fuzzers and then runs said fuzzers, [filing bugs](https://issues.oss-fuzz.com/issues?q=status:open%20componentid:1638179%20project:skia).

The Skia-specific code to build the fuzzers is found in [oss-fuzz/projects/skia](https://github.com/google/oss-fuzz/tree/master/projects/skia)
and the build status can be found [here](https://oss-fuzz-build-logs.storage.googleapis.com/index.html#skia).
When everything is working smoothly, the version of Skia that is fuzzed should be updated about
2/day.

See <https://skia.googlesource.com/skia/+/refs/heads/main/fuzz/README.md> for more details.

If there are issues with oss-fuzz, file them against [component 491058](http://b/issues?q=componentid:491058) [googler only].
