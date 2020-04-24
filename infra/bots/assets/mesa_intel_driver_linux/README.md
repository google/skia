Creating the Mesa Intel Linux driver asset (which supports Vulkan)
============================================================

Using the automated asset python scripts requires that Docker be installed.

    mesa_intel_driver_linux$ python create_and_upload.py

See below for steps on just building the driver.

Using Docker
------------
It is easiest to just use the pre-built docker image.

    docker run -v /tmp/out:/OUT -e MESA_VERSION=18.1.7 gcr.io/skia-public/mesa-driver-builder:latest /opt/build_mesa.sh

You may change `/tmp/out` to be the desired output directory and `18.1.7` to be the desired
version of the mesa driver.

Finally, use `/tmp/out` as the input directory to the upload script.

Building it manually
--------------------
If Docker is not installed, these steps may be used to build the driver.
This is known to work on Ubuntu 18.04, but is stale since we use the Docker container
for day-to-day builds.

Install all deps

    sudo apt-get install autoconf libtool scons flex bison llvm-dev libpthread-stubs0-dev x11proto-gl-dev libdrm-dev libdrm2 x11proto-dri2-dev x11proto-dri3-dev x11proto-present-dev libxcb1-dev libxcb-dri3-dev libxcb-present-dev libxshmfence-dev xserver-xorg-core xserver-xorg-dev x11proto-xext-dev libxext-dev libxdamage-dev libx11-xcb-dev libxcb-glx0-dev libxcb-dri2-0-dev libva-dev libomxil-bellagio-dev

    sudo pip install mako

The following steps are also represented in `mesa-driver-builder/build_mesa.sh`

Get the source from ftp.freedesktop.org/pub/mesa/

    MESA_VERSION=18.1.7
    wget ftp://ftp.freedesktop.org/pub/mesa/mesa-$MESA_VERSION.tar.gz
    gunzip mesa-$MESA_VERSION.tar.gz
    tar --extract -f mesa-$MESA_VERSION.tar
    mv mesa-$MESA_VERSION/ mesa
    cd mesa


Build the driver

    # For the debug resource, use --enable-debug
    mesa$ ./autogen.sh --disable-radeon --with-gallium-drivers=i915 --with-vulkan-drivers=intel
    mesa$ make -j 50


Tweak icd.json file and output dir (mesa/lib)

    mesa$ cp src/intel/vulkan/intel_icd.x86_64.json lib/
    # modify the pathname in the intel_icd.x86_64.json file to be ./libvulkan_intel.so
    mesa$ rm -rf lib/gallium  # We don't need this
    mesa$ rm lib/nouveau_vieux_dri.so lib/r200_dri.so lib/radeon_dri.so # We don't need these

Finally, use mesa/lib as the input directory to the upload script.


Docker Image Maintanence
------------------------
The docker image `mesa-driver-builder` is an Ubuntu container with many build
tools installed (including Clang 6). It is designed specifically to build the mesa driver.

The image only needs to be re-built when the dependencies change or the build_mesa.sh is updated.

    docker build -t mesa-driver-builder ./mesa-driver-builder/
    # use v1, v2, v3, etc to handle changes/updates to the image.
    docker tag mesa-driver-builder gcr.io/skia-public/mesa-driver-builder:v1
    docker push gcr.io/skia-public/mesa-driver-builder:v1