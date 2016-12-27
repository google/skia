How to download Skia
====================

Set up Python 2
---------------

Install Python version 2 if your system doesn't have it. If the output of

    python -V

has the form 3.X.X, you'll need to make python commands refer to the earlier
release when using gclient, building Skia, or running scripts in the source
tree. In Unix-like terminals,

    ln -s /path/to/python2 /path/to/new/alias/python
    ln -s /path/to/python2-config /path/to/new/alias/python-config
    export PATH="path/to/new/alias:${PATH}"

will make links that your system can find. They can be deleted later like files.

Install depot_tools and Git
---------------------------

Follow the instructions on [Installing Chromium's
depot_tools](http://www.chromium.org/developers/how-tos/install-depot-tools)
to download depot_tools (which includes gclient, git-cl, and Ninja).

    git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
    export PATH="${PWD}/depot_tools:${PATH}"

depot_tools will also install Git on your system, if it wasn't installed
already.

Clone the Skia repository
-------------------------

    mkdir $SRC
    cd $SRC
    gclient config --unmanaged https://skia.googlesource.com/skia.git
    gclient sync
    cd skia

Getting started with Skia
-------------------------

You probably now want to [build](./build) Skia.

Changing and contributing to Skia
---------------------------------

At this point, you have everything you need to build and use Skia!  If
you want to make changes, and possibly contribute them back to the Skia
project, read [How To Submit a Patch](../dev/contrib/submit).
