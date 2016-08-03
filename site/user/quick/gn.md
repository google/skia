GN
=====

[GN](https://chromium.googlesource.com/chromium/src/tools/gn/)
is a new meta-build system originally designed to replace GYP in Chromium.

You can build Skia using GN in a limited number of configurations.  We expect
that as that limited number rises, GN will become the preferred, and then only,
way to build Skia.

Supported Features
----------

    * Linux, Mac
    * Software and GL rendering
    * libskia.a, libskia.so
    * DM, nanobench

Quickstart
----------

Please check out Skia using the instructions in one of the other quick start
guides.  We diverge where they'd first run some command with "gyp" in it.

<!--?prettify lang=sh?-->

    # After gclient sync, run fetch-gn to make sure you have GN.
    gclient sync && bin/fetch-gn

    # Run GN to generate your build files.  Some examples.
    gn gen out/Debug
    gn gen out/Release --args=is_debug=false
    gn gen out/Clang --args='cc="clang" cxx="clang++"'
    gn gen out/Shared --args=is_component_build=true

    # Build
    ninja -C out/Release
    ninja -C out/Debug
    ninja -C out/Clang
    ninja -C out/Shared

From here everything is pretty much business as usual.
