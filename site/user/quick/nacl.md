NaCl (Experimental)
===================

Important Notes
---------------

  * This process has only been verified to work on Linux
  * Skia for NaCl is new and currently under development.  Therefore, some features are not (yet) supported:
    * GPU backend
    * Fonts - Currently, NaCl has no way to access system fonts.  This means
      that text drawn in Skia will not display.  A Pepper font API is in the
      works and should be available in the near future, but for now your best
      bet is to either package font data with your nexe or to send font data
      from javascript to your plugin at runtime.  Note that this will be the
      case with any graphics library in NaCl until the font API is finished.

Prerequisites
-------------

Execute the following commands in whatever directory you want to hold the NaCl SDK directory:

    wget http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip
    unzip nacl_sdk.zip
    rm nacl_sdk.zip
    nacl_sdk/naclsdk update pepper_32
    export NACL_SDK_ROOT=/path/to/nacl_sdk

Check out the Skia source
-------------------------

We use the "gclient" script (part of the depot_tools toolkit) to manage the
Skia source code. Follow the instructions at
http://www.chromium.org/developers/how-tos/depottools to get the gclient
script from depot_tools.

Instead of checking out trunk directly you will use gclient to checkout the
nacl directory, which will automatically pull the trunk directory for you.
Execute the following commands in whatever directory you want to be the root
for your Skia on NaCl development:

    gclient config https://skia.googlesource.com/skia.git
    gclient sync

Building the Skia libraries for NaCl
------------------------------------

The nacl_make script is used to compile Skia targets for NaCl.  It sets the
appropriate environment variables, calls GYP to generate makefiles, and runs
Make to build both 32 and 64-bit targets as required by NaCl.  To build the
Skia libraries, run the following from the trunk directory:

    platform_tools/nacl/nacl_make skia_lib

This will result in a set of static libraries being built into the out/nacl32
and out/nacl64 directories.  You can use these libraries in external NaCl
apps.

Building and running Skia's Apps in NaCl (Experimental)
-------------------------------------------------------

It is possible to run some of Skia's console apps in the browser.

### Skia Unit Tests

Build Skia tests from the trunk directory:

    platform_tools/nacl/bin/nacl_make tests

This will build the tests executable.  We include a tiny HTTP server (borrowed
from the NaCl SDK) in order to run the apps:

    cd platform_tools/nacl
    ./httpd.py

The HTTP server runs on port 5103 by default.  In Chrome, navigate to
`http://localhost:5103/` and click on the link for "unit tests."  After the
module downloads, you should see the tests begin to run.

### Sample App

The sample app relies on the GPU backend.  Therefore, it will compile but will not yet run.

    platform_tools/nacl/bin/nacl_make SampleApp

You can access the sample app at http://localhost:5103/SampleApp.

### Debugger

The debugger is currently in a partially-working state:

    platform_tools/nacl/bin/nacl_make debugger

You can access it at http://localhost:5103/debugger.
