How to download Skia
====================

Install depot_tools and Git
---------------------------

Follow the instructions on [Installing Chromium's
depot_tools](http://www.chromium.org/developers/how-tos/install-depot-tools)
to download depot_tools (which includes gclient, git-cl, and Ninja).

<!--?prettify lang=sh?-->

    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"

depot_tools will also install Git on your system, if it wasn't installed
already.


Configure Git
-------------

<!--?prettify lang=sh?-->

    git config --global user.name "Your Name"
    git config --global user.email you@example.com

Clone the Skia repository
-------------------------

<!--?prettify lang=sh?-->

    git clone https://skia.googlesource.com/skia.git
    cd skia

Get Skia's dependencies and generate Ninja build files
------------------------------------------------------

<!--?prettify lang=sh?-->

    python bin/sync-and-gyp

<!--
    python tools/git-sync-deps
    python ./gyp_skia
-->

Compile all default targets
---------------------------

<!--?prettify lang=sh?-->

    ninja -C out/Debug

Execute Skia tests
------------------

[More about Skia correctness testing tools](../dev/testing/testing)

<!--?prettify lang=sh?-->

    out/Debug/dm

Execute Skia sample application
-------------------------------

[More about Skia's SampleApp](sample/sampleapp)

<!--?prettify lang=sh?-->

    out/Debug/SampleApp

At this point, you have everything you need to build and use Skia!  If
you want to make changes, and possibly contribute them back to the Skia
project, read [How To Submit a Patch](../dev/contrib/submit).
