Linux
=====

Quickstart
----------

<!--?prettify lang=sh?-->


    # Install depot_tools (this provides ninja and git-cl).
    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"

    # Get Skia.
    git clone 'https://skia.googlesource.com/skia'
    cd skia

    # Install Dependencies (may require sudo).
    tools/install_dependencies.sh

    # Sync, Config, and Build.
    bin/sync-and-gyp
    ninja -C out/Debug dm SampleApp

    # Run DM, the Skia test app.
    out/Debug/dm

    # Run SampleApp.
    out/Debug/SampleApp

    # Run nanobench, the Skia benchmarking tool.
    ninja -C out/Release nanobench
    out/Release/nanobench


<a name="prerequisites"></a>Prerequisites
-----------------------------------------

On a Ubuntu 12.04 (Precise) or Ubuntu 14.04 (Trusty) system, you can run
`tools/install_dependencies.sh`, which will install the needed packages.  On
Ubuntu 12.04, you will need to install the`ninja` build tool separately, which
comes with Chromium's `depot_tools`.

To contribute changes back to Skia, you will need `git-cl`, which
comes with Chromium's `depot_tools`.

(If you use another Linux distribution, please consider contributing back
instructions for installing the required packages — we can then incorporate
that knowledge into the `tools/install_dependencies.sh` tool.)

Make sure the following have been installed:

*   [Chromium depot_tools](http://www.chromium.org/developers/how-tos/depottools)

*   A C++ compiler (typically GCC or Clang) **build-essential** or **clang-3.6**

*   Python 2.7.x: **python2.7-dev**

*   The FreeType and Fontconfig font engines: **libfreetype6-dev** and
    **libfontconfig1-dev**

*   Mesa OpenGL utility library headers: **libglu1-mesa-dev**

*   Mesa headers: **mesa-common-dev**

*   GL, such as **freeglut3-dev**

*   QT4, used by the [Skia Debugger](/dev/tools/debugger): **libqt4-dev**

Check out the source code
-------------------------

Follow the instructions [here](../download) for downloading the Skia source.

Notes
-----

1.  On 32-bit Linux (when `uname -m` is *not* `x86_64`), you will have to
    explicitly specify the architecture:

    <!--?prettify lang=sh?-->

        GYP_DEFINES='skia_arch_type=x86' python bin/sync-and-gyp
2.  By default, many Linux systems use gcc by default.  To use clang you will
need to [set the CC and CXX environment variables](/user/tips#gypdefines).

Generate build files
--------------------

See  [this page for generating build files and run tests](desktop).
