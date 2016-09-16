GN
=====

[GN](https://chromium.googlesource.com/chromium/src/tools/gn/)
is a new meta-build system originally designed to replace GYP in Chromium.

You can build Skia using GN in a limited number of configurations.  We expect
that as that limited number rises, GN will become the preferred, and then only,
way to build Skia.

Supported Features
----------

    * Linux, Mac, Android
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

    # Run GN to generate your build files.
    gn gen out/Static --args='is_official_build=true'
    gn gen out/Shared --args='is_official_build=true is_component_build=true'

    # GN allows fine-grained settings for developers and special situations.
    gn gen out/Debug
    gn gen out/Release  --args='is_debug=false'
    gn gen out/Clang    --args='cc="clang" cxx="clang++"'
    gn gen out/Cached   --args='compiler_prefix="ccache"'
    gn gen out/RTTI     --args='extra_cflags_cc="-frtti"'

    # To see all the current GN arguments, run
    gn args out/Debug --list

    # Build
    ninja -C out/Static
    ninja -C out/Shared
    ninja -C out/Debug
    ninja -C out/Release
    ninja -C out/Clang
    ninja -C out/Cached
    ninja -C out/RTTI

From here everything is pretty much business as usual.

Android
-------

To build Skia for Android you need an [Android
NDK](https://developer.android.com/ndk/index.html).

If you do not have an NDK and have access to CIPD, you
can use one of these commands to fetch the NDK our bots use:

<!--?prettify lang=sh?-->

    python infra/bots/assets/android_ndk_linux/download.py  -t /tmp/ndk
    python infra/bots/assets/android_ndk_darwin/download.py -t /tmp/ndk

When generating your GN build files, pass the path to your `ndk` and your
desired `target_cpu`:

<!--?prettify lang=sh?-->

    gn gen out/arm      --args='ndk="/tmp/ndk" target_cpu="arm"'
    gn gen out/arm64    --args='ndk="/tmp/ndk" target_cpu="arm64"'
    gn gen out/mips64el --args='ndk="/tmp/ndk" target_cpu="mips64el"'
    gn gen out/mipsel   --args='ndk="/tmp/ndk" target_cpu="mipsel"'
    gn gen out/x64      --args='ndk="/tmp/ndk" target_cpu="x64"'
    gn gen out/x86      --args='ndk="/tmp/ndk" target_cpu="x86"'

Other arguments like `is_debug` and `is_component_build` continue to work.
Tweaking `ndk_api` gives you access to newer Android features like Vulkan.

To test on a locally connected Android device, you can use our `droid` convenience script:

<!--?prettify lang=sh?-->

    ninja -C out/arm64
    bin/droid out/arm64/dm --src gm --config gpu
