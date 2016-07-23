Running with Memory Sanitizer
=============================

Prerequisites
-------------

The msan build builds Clang from scratch, so you need to download Clang first:

    bin/sync-and-gyp --deps=llvm

Build
-----

    tools/xsan_build memory dm

Run
---

    out/Debug/dm -v --match ~Codec ~BlurLargeImage ~FontMgrAndroidParser
