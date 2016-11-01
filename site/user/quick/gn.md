GN
=====

[GN](https://chromium.googlesource.com/chromium/src/tools/gn/)
is a new meta-build system originally designed to replace GYP in Chromium.

You can build Skia using GN in a limited number of configurations.  We expect
that as that limited number rises, GN will become the preferred, and then only,
way to build Skia.

Supported Features
----------

    * Linux, Mac, Android, Windows
    * Software, GL, Vulkan rendering
    * libskia.a, libskia.so
    * DM, nanobench, a few other tools
    * (Pretty much everything but iOS and some complicated tools.)

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
    gn gen out/Cached   --args='cc_wrapper="ccache"'
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
    python infra/bots/assets/android_ndk_windows/download.py -t C:/ndk

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

To test on an Android device, push the binary and `resources` over,
and run it as normal.  You may find `bin/droid` convenient.

<!--?prettify lang=sh?-->

    ninja -C out/arm64
    adb push out/arm64/dm /data/local/tmp
    adb push resources /data/local/tmp
    adb shell "cd /data/local/tmp; ./dm --src gm --config gpu"

Mac
---

Mac users may want to pass `--ide=xcode` to `gn gen` to generate an Xcode project.

Windows
-------

Skia should build on Windows with Visual Studio 2015 Update 3.  No other
version, older or newer, is supported.  If you use Visual Studio, you may
want to pass `--ide=vs` to `gn gen` to generate `all.sln`.

The bots use a packaged toolchain, which you may be able to download like this:

<!--?prettify lang=sh?-->

    python infra/bots/assets/win_toolchain/download.py -t C:/toolchain

If you pass that downloaded path to GN via `windk`, you can build using that
toolchain instead of your own from Visual Studio.  This toolchain is the only
way we support 32-bit builds, by also setting `target_cpu="x86"`.

CMake
-----

We have added a GN-to-CMake translator mainly for use with IDEs that like CMake
project descriptions.  This is not meant for any purpose beyond development.

<!--?prettify lang=sh?-->

    gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py

Third-party Dependencies
------------------------

Skia offers several features that make use of third-party libraries, like
libpng, libwebp, or libjpeg-turbo to decode images, or ICU and sftnly to subset
fonts.  All these third-party dependencies are optional, and can be controlled
by a GN argument that looks something like `skia_use_foo` for appropriate
`foo`.

Most of these third-party dependencies can also be satisfied by pre-built
system libraries.  If `skia_use_foo` is enabled, turn on `skia_use_system_foo`
to build and link Skia against the headers and libaries found on the normal
system paths.  Remember, you can use `extra_cflags` and `extra_ldflags` to add
include or library paths if needed.

By default Skia will attempt to build and embed its own copies of these
third-party libraries.  This configuration is for development and testing only.
We do not recommend shipping Skia this way.  Note however, this is the only
configuration of Skia that receives significant testing.
