How to build Skia
=================

Make sure you have first followed the [instructions to download
Skia](./download).

Skia uses [GN](https://chromium.googlesource.com/chromium/src/tools/gn/) to
configure its builds.

A few build configurations remain unported to GN, so you may see some `.gyp`
files laying around left over from when we used GYP.  Don't bother looking at
them.

The build process is only officially tested on Windows 10, Ubuntu 14.04, and Mac
10.12. GCC 5 and 6 especially may not work with everything as is.

Quickstart
----------

After gclient sync, run `fetch-gn` inside the skia source directory to make sure
you have GN.

    gclient sync && python bin/fetch-gn

Run GN to generate your build files.

    gn gen out/Static --args='is_official_build=true'
    gn gen out/Shared --args='is_official_build=true is_component_build=true'

GN allows fine-grained settings for developers and special situations.

    gn gen out/Debug
    gn gen out/Release  --args='is_debug=false'
    gn gen out/Clang    --args='cc="clang" cxx="clang++"'
    gn gen out/Cached   --args='cc_wrapper="ccache"'
    gn gen out/RTTI     --args='extra_cflags_cc=["-frtti"]'

To see all the arguments available, you can run

    gn args out/Debug --list

Having generated your build files, run Ninja to compile and link Skia.

    ninja -C out/Static
    ninja -C out/Shared
    ninja -C out/Debug
    ninja -C out/Release
    ninja -C out/Clang
    ninja -C out/Cached
    ninja -C out/RTTI

Ninja's help documents other features.

    ninja -C out/Clang skia

(re)computes enough to create a static library. To see all such targets, run

    ninja -C out/Clang -t targets

for a list, which comes from the `gn/BUILD.gn` file.

Android
-------

To build Skia for Android you need an [Android
NDK](https://developer.android.com/ndk/index.html).

If you do not have an NDK and have access to CIPD, you
can use one of these commands to fetch the NDK our bots use:

    python infra/bots/assets/android_ndk_linux/download.py  -t /tmp/ndk
    python infra/bots/assets/android_ndk_darwin/download.py -t /tmp/ndk
    python infra/bots/assets/android_ndk_windows/download.py -t C:/ndk

When generating your GN build files, pass the path to your `ndk` and your
desired `target_cpu`:

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

    ninja -C out/arm64
    adb push out/arm64/dm /data/local/tmp
    adb push resources /data/local/tmp
    adb shell "cd /data/local/tmp; ./dm --src gm --config gpu"

Mac
---

Mac users may want to pass `--ide=xcode` to `gn gen` to generate an Xcode project.

Windows
-------

Skia can build on Windows with Visual Studio 2015 Update 3.  No older or newer
version is supported. The bots use a packaged toolchain, which you may be able
to download like this:

    python infra/bots/assets/win_toolchain/download.py -t C:/toolchain

If you pass that downloaded path to GN via `windk`, you can build using that
toolchain instead of your own from Visual Studio.  This toolchain is the only
way we support 32-bit builds, by also setting `target_cpu="x86"`.

### Visual Studio Solutions

If you use Visual Studio, you may want to pass `--ide=vs` to `gn gen` to
generate `all.sln`.  That solution will exist within the GN directory for the
specific configuration, and will only build/run that configuration.

If you want a Visual Studio Solution that supports multiple GN configurations,
there is a helper script. It requires that all of your GN directories be inside
the `out` directory. First, create all of your GN configurations as usual.
Pass `--ide=vs` when running `gn gen` for each one. Then:

    python gn/gn_meta_sln.py

This creates a new dedicated output directory and solution file
`out/sln/skia.sln`. It has one solution configuration for each GN configuration,
and supports building and running any of them. It also adjusts syntax highlighting
of inactive code blocks based on preprocessor definitions from the selected
solution configuration.

CMake
-----

We have added a GN-to-CMake translator mainly for use with IDEs that like CMake
project descriptions.  This is not meant for any purpose beyond development.

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
to build and link Skia against the headers and libaries found on the system
paths.  You can use `extra_cflags` and `extra_ldflags` to add include or
library paths if needed.

By default Skia will build and embed its own copies of these third-party
libraries.  This configuration is for development only.  We do not recommend
shipping Skia this way.  However, this is the only configuration of Skia that
receives significant testing.
