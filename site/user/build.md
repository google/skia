How to build Skia
=================

Make sure you have first followed the [instructions to download
Skia](./download).

Skia uses [GN](https://chromium.googlesource.com/chromium/src/tools/gn/) to
configure its builds.

`is_official_build` and Third-party Dependencies
------------------------------------------------

Most users of Skia should set `is_official_build=true`, and most developers
should leave it to its `false` default.

This mode configures Skia in a way that's suitable to ship: an optimized build
with no debug symbols, dynamically linked against its third-party dependencies
using the ordinary library search path.

In contrast, the developer-oriented default is an unoptimized build with full
debug symbols and all third-party dependencies built from source and embedded
into libskia.  This is how do all our manual and automated testing.

Skia offers several features that make use of third-party libraries, like
libpng, libwebp, or libjpeg-turbo to decode images, or ICU and sftnly to subset
fonts.  All these third-party dependencies are optional and can be controlled
by a GN argument that looks something like `skia_use_foo` for appropriate
`foo`.

If `skia_use_foo` is enabled, enabling `skia_use_system_foo` will build and
link Skia against the headers and libaries found on the system paths.
`is_official_build=true` enables all `skia_use_system_foo` by default.  You can
use `extra_cflags` and `extra_ldflags` to add include or library paths if
needed.

Quickstart
----------

Run GN to generate your build files.

    bin/gn gen out/Static --args='is_official_build=true'
    bin/gn gen out/Shared --args='is_official_build=true is_component_build=true'

If you find you don't have `bin/gn`, make sure you've run

    python tools/git-sync-deps

GN allows fine-grained settings for developers and special situations.

    bin/gn gen out/Debug
    bin/gn gen out/Release  --args='is_debug=false'
    bin/gn gen out/Clang    --args='cc="clang" cxx="clang++"'
    bin/gn gen out/Cached   --args='cc_wrapper="ccache"'
    bin/gn gen out/RTTI     --args='extra_cflags_cc=["-frtti"]'

To see all the arguments available, you can run

    bin/gn args out/Debug --list

Having generated your build files, run Ninja to compile and link Skia.

    ninja -C out/Static
    ninja -C out/Shared
    ninja -C out/Debug
    ninja -C out/Release
    ninja -C out/Clang
    ninja -C out/Cached
    ninja -C out/RTTI

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

    bin/gn gen out/arm      --args='ndk="/tmp/ndk" target_cpu="arm"'
    bin/gn gen out/arm64    --args='ndk="/tmp/ndk" target_cpu="arm64"'
    bin/gn gen out/mips64el --args='ndk="/tmp/ndk" target_cpu="mips64el"'
    bin/gn gen out/mipsel   --args='ndk="/tmp/ndk" target_cpu="mipsel"'
    bin/gn gen out/x64      --args='ndk="/tmp/ndk" target_cpu="x64"'
    bin/gn gen out/x86      --args='ndk="/tmp/ndk" target_cpu="x86"'

Other arguments like `is_debug` and `is_component_build` continue to work.
Tweaking `ndk_api` gives you access to newer Android features like Vulkan.

To test on an Android device, push the binary and `resources` over,
and run it as normal.  You may find `bin/droid` convenient.

    ninja -C out/arm64
    adb push out/arm64/dm /data/local/tmp
    adb push resources /data/local/tmp
    adb shell "cd /data/local/tmp; ./dm --src gm --config gl"

Mac
---

Mac users may want to pass `--ide=xcode` to `bin/gn gen` to generate an Xcode project.

iOS
---

Run GN to generate your build files.  Set `target_os="ios"` to build for iOS.
This defaults to `target_cpu="arm64"`.  Choosing `x64` targets the iOS simulator.

    bin/gn gen out/ios64  --args='target_os="ios"'
    bin/gn gen out/ios32  --args='target_os="ios" target_cpu="arm"'
    bin/gn gen out/iossim --args='target_os="ios" target_cpu="x64"'

Googlers who want to sign and run iOS test binaries can do so by running something like

    python gn/package_ios.py out/Debug/dm
    python gn/package_ios.py out/Release/nanobench

These commands will create and sign `dm.app` or `nanobench.app` packages you
can push to iOS devices registered for Google development.  `ios-deploy` makes
installing and running these packages easy:

    ios-deploy -b out/Debug/dm.app -d --args "--match foo"

If you find yourself missing a Google signing identity or provisioning profile,
you'll want to have a read through go/appledev.

Windows
-------

Skia can build on Windows with Visual Studio 2015 Update 3, or Visual Studio
2017 by setting `msvc = 2017` in GN.  No older versions are supported. The bots
use a packaged 2015 toolchain, which Googlers can download like this:

    python infra/bots/assets/win_toolchain/download.py -t C:/toolchain

If you pass that downloaded path to GN via `windk`, you can build using that
toolchain instead of your own from Visual Studio.  This toolchain is the only
way we support 32-bit builds with 2015, by also setting `target_cpu="x86"`.
32-bit builds should work with the default 2017 install if you follow the
directions GN prints to set up your environment.

### Visual Studio Solutions

If you use Visual Studio, you may want to pass `--ide=vs` to `bin/gn gen` to
generate `all.sln`.  That solution will exist within the GN directory for the
specific configuration, and will only build/run that configuration.

If you want a Visual Studio Solution that supports multiple GN configurations,
there is a helper script. It requires that all of your GN directories be inside
the `out` directory. First, create all of your GN configurations as usual.
Pass `--ide=vs` when running `bin/gn gen` for each one. Then:

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

    bin/gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py
