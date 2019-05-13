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
into libskia.  This is how we do all our manual and automated testing.

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

A note on software backend performance
--------------------------------------

A number of routines in Skia's software backend have been written to run
fastest when compiled by Clang.  If you depend on software rasterization, image
decoding, or color space conversion and compile Skia with GCC, MSVC or another
compiler, you will see dramatically worse performance than if you use Clang.

This choice was only a matter of prioritization; there is nothing fundamentally
wrong with non-Clang compilers.  So if this is a serious issue for you, please
let us know on the mailing list.

Quickstart
----------

Run GN to generate your build files.

    bin/gn gen out/Static --args='is_official_build=true'
    bin/gn gen out/Shared --args='is_official_build=true is_component_build=true'

If you find you don't have `bin/gn`, make sure you've run

    python2 tools/git-sync-deps

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

If some header files are missing, install the corresponding dependencies

    tools/install_dependencies.sh

Android
-------

To build Skia for Android you need an [Android
NDK](https://developer.android.com/ndk/index.html).

If you do not have an NDK and have access to CIPD, you
can use one of these commands to fetch the NDK our bots use:

    python2 infra/bots/assets/android_ndk_linux/download.py  -t /tmp/ndk
    python2 infra/bots/assets/android_ndk_darwin/download.py -t /tmp/ndk
    python2 infra/bots/assets/android_ndk_windows/download.py -t C:/ndk

When generating your GN build files, pass the path to your `ndk` and your
desired `target_cpu`:

    bin/gn gen out/arm   --args='ndk="/tmp/ndk" target_cpu="arm"'
    bin/gn gen out/arm64 --args='ndk="/tmp/ndk" target_cpu="arm64"'
    bin/gn gen out/x64   --args='ndk="/tmp/ndk" target_cpu="x64"'
    bin/gn gen out/x86   --args='ndk="/tmp/ndk" target_cpu="x86"'

Other arguments like `is_debug` and `is_component_build` continue to work.
Tweaking `ndk_api` gives you access to newer Android features like Vulkan.

To test on an Android device, push the binary and `resources` over,
and run it as normal.  You may find `bin/droid` convenient.

    ninja -C out/arm64
    adb push out/arm64/dm /data/local/tmp
    adb push resources /data/local/tmp
    adb shell "cd /data/local/tmp; ./dm --src gm --config gl"


ChromeOS
--------------
To cross-compile Skia for arm ChromeOS devices the following is needed:

  - Clang 4 or newer
  - An armhf sysroot
  - The (E)GL lib files on the arm chromebook to link against.

To compile Skia for an x86 ChromeOS device, one only needs Clang and the lib files.

If you have access to CIPD, you can fetch all of these as follows:

    python2 infra/bots/assets/clang_linux/download.py  -t /opt/clang
    python2 infra/bots/assets/armhf_sysroot/download.py -t /opt/armhf_sysroot
    python2 infra/bots/assets/chromebook_arm_gles/download.py -t /opt/chromebook_arm_gles
    python2 infra/bots/assets/chromebook_x86_64_gles/download.py -t /opt/chromebook_x86_64_gles

If you don't have authorization to use those assets, then see the README.md files for
[armhf_sysroot](https://skia.googlesource.com/skia/+/master/infra/bots/assets/armhf_sysroot/README.md),
[chromebook_arm_gles](https://skia.googlesource.com/skia/+/master/infra/bots/assets/chromebook_arm_gles/README.md), and
[chromebook_x86_64_gles](https://skia.googlesource.com/skia/+/master/infra/bots/assets/chromebook_x86_64_gles/README.md)
for instructions on creating those assets.

Once those files are in place, generate the GN args that resemble the following:

    #ARM
    cc= "/opt/clang/bin/clang"
    cxx = "/opt/clang/bin/clang++"

    extra_asmflags = [
        "--target=armv7a-linux-gnueabihf",
        "--sysroot=/opt/armhf_sysroot/",
        "-march=armv7-a",
        "-mfpu=neon",
        "-mthumb",
    ]
    extra_cflags=[
        "--target=armv7a-linux-gnueabihf",
        "--sysroot=/opt/armhf_sysroot",
        "-I/opt/chromebook_arm_gles/include",
        "-I/opt/armhf_sysroot/include/",
        "-I/opt/armhf_sysroot/include/c++/4.8.4/",
        "-I/opt/armhf_sysroot/include/c++/4.8.4/arm-linux-gnueabihf/",
        "-DMESA_EGL_NO_X11_HEADERS",
        "-funwind-tables",
    ]
    extra_ldflags=[
        "--sysroot=/opt/armhf_sysroot",
        "-B/opt/armhf_sysroot/bin",
        "-B/opt/armhf_sysroot/gcc-cross",
        "-L/opt/armhf_sysroot/gcc-cross",
        "-L/opt/armhf_sysroot/lib",
        "-L/opt/chromebook_arm_gles/lib",
        "--target=armv7a-linux-gnueabihf",
    ]
    target_cpu="arm"
    skia_use_fontconfig = false
    skia_use_system_freetype2 = false
    skia_use_egl = true


    # x86_64
    cc= "/opt/clang/bin/clang"
    cxx = "/opt/clang/bin/clang++"
    extra_cflags=[
        "-I/opt/clang/include/c++/v1/",
        "-I/opt/chromebook_x86_64_gles/include",
        "-DMESA_EGL_NO_X11_HEADERS",
        "-DEGL_NO_IMAGE_EXTERNAL",
    ]
    extra_ldflags=[
        "-stdlib=libc++",
        "-fuse-ld=lld",
        "-L/opt/chromebook_x86_64_gles/lib",
    ]
    target_cpu="x64"
    skia_use_fontconfig = false
    skia_use_system_freetype2 = false
    skia_use_egl = true

Compile dm (or another executable of your choice) with ninja, as per usual.

Push the binary to a chromebook via ssh and [run dm as normal](https://skia.org/dev/testing/tests)
using the gles GPU config.

Most chromebooks by default have their home directory partition marked as noexec.
To avoid "permission denied" errors, remember to run something like:

    sudo mount -i -o remount,exec /home/chronos

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

This will also package (and for devices, sign) iOS test binaries. This defaults to a
Google signing identity and provisioning profile. To use a different one set `skia_ios_identity`
to match your code signing identity and `skia_ios_profile` to the name of your provisioning
profile, e.g. `skia_ios_identity=".*Jane Doe.*" skia_ios_profile="iPad Profile"`. A list of
identities can be found by typing `security find-identity` on the command line. The name of the
provisioning profile should be available on the Apple Developer site.

For signed packages `ios-deploy` makes installing and running them on a device easy:

    ios-deploy -b out/Debug/dm.app -d --args "--match foo"

Alternatively you can generate an Xcode project by passing `--ide=xcode` to `bin/gn gen`.

If you find yourself missing a Google signing identity or provisioning profile,
you'll want to have a read through go/appledev.

Deploying to a device with an OS older than the current SDK doesn't currently work through Xcode,
but can be done on the command line by setting the environment variable IPHONEOS_DEPLOYMENT_TARGET
to the desired OS version.

Windows
-------

Skia can build on Windows with Visual Studio 2017 or Visual Studio 2015 Update 3.
If GN is unable to locate either of those, it will print an error message. In that
case, you can pass your `VC` path to GN via `win_vc`.

Skia can be compiled with the free [Build Tools for Visual Studio
2017](https://www.visualstudio.com/downloads/#build-tools-for-visual-studio-2017).

The bots use a packaged 2017 toolchain, which Googlers can download like this:

    python2 infra/bots/assets/win_toolchain/download.py -t C:/toolchain

You can then pass the VC and SDK paths to GN by setting your GN args:

    win_vc = "C:\toolchain\VC"
    win_sdk = "C:\toolchain\win_sdk"

This toolchain is the only way we support 32-bit builds, by also setting `target_cpu="x86"`.
There is also a corresponding 2015 toolchain, downloaded via `infra/bots/assets/win_toolchain_2015`.

The Skia build assumes that the PATHEXT environment variable contains ".EXE".

### **Highly Recommended**: Build with clang-cl

Skia uses generated code that is only optimized when Skia is built with clang. Other compilers get generic
unoptimized code.

Setting the `cc` and `cxx` gn args is _not_ sufficient to build with clang-cl. These variables
are ignored on Windows. Instead set the variable `clang_win` to your LLVM installation directory.
If you installed the prebuilt LLVM downloaded from [here](https://releases.llvm.org/download.html "LLVM Download") in the default location that would be:

    clang_win = "C:\Program Files\LLVM"

Follow the standard Windows path specification and not MinGW convention (e.g. `C:\Program Files\LLVM` not ~~`/c/Program Files/LLVM`~~).

### Visual Studio Solutions

If you use Visual Studio, you may want to pass `--ide=vs` to `bin/gn gen` to
generate `all.sln`.  That solution will exist within the GN directory for the
specific configuration, and will only build/run that configuration.

If you want a Visual Studio Solution that supports multiple GN configurations,
there is a helper script. It requires that all of your GN directories be inside
the `out` directory. First, create all of your GN configurations as usual.
Pass `--ide=vs` when running `bin/gn gen` for each one. Then:

    python2 gn/gn_meta_sln.py

This creates a new dedicated output directory and solution file
`out/sln/skia.sln`. It has one solution configuration for each GN configuration,
and supports building and running any of them. It also adjusts syntax highlighting
of inactive code blocks based on preprocessor definitions from the selected
solution configuration.

Windows ARM64
-------------

There is early, experimental support for [Windows 10 on ARM](https://docs.microsoft.com/en-us/windows/arm/).
This currently requires (a recent version of) MSVC, and the `Visual C++ compilers and libraries for ARM64`
individual component in the Visual Studio Installer. For Googlers, the win_toolchain asset includes the
ARM64 compiler.

To use that toolchain, set the `target_cpu` GN argument to `"arm64"`. Note that OpenGL is not supported
by Windows 10 on ARM, so Skia's GL backends are stubbed out, and will not work. ANGLE is supported:

    bin/gn gen out/win-arm64 --args='target_cpu="arm64" skia_use_angle=true'

This will produce a build of Skia that can use the software or ANGLE backends, in DM. Viewer only works
when launched with `--backend angle`, because the software backend tries to use OpenGL to display the
window contents.

CMake
-----

We have added a GN-to-CMake translator mainly for use with IDEs that like CMake
project descriptions.  This is not meant for any purpose beyond development.

    bin/gn gen out/config --ide=json --json-ide-script=../../gn/gn_to_cmake.py
