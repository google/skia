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

Clone the Skia repository
-------------------------

<!--?prettify lang=sh?-->

    git clone https://skia.googlesource.com/skia.git
    cd skia
    python tools/git-sync-deps

Getting started with Skia
-------------------------

You probably now want to [build](./build) Skia.

Changing and contributing to Skia
---------------------------------

At this point, you have everything you need to build and use Skia!  If
you want to make changes, and possibly contribute them back to the Skia
project, read [How To Submit a Patch](../dev/contrib/submit).
