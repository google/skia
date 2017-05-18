Creating the Linux Vulkan driver for Intel

This is known to work on an Intel machine running Ubuntu 16.10.

Install all deps

    sudo apt-get install autoconf libtool scons flex bison llvm-dev libpthread-stubs0-dev x11proto-gl-dev libdrm-dev x11proto-dri2-dev x11proto-dri3-dev x11proto-present-dev libxcb1-dev libxcb-dri3-dev libxcb-present-dev libxshmfence-dev xserver-xorg-core xserver-xorg-dev x11proto-xext-dev libxext-dev libxdamage-dev libx11-xcb-dev libxcb-glx0-dev libxcb-dri2-0-dev

    sudo pip install mako


Get the source from ftp.freedesktop.org/pub/mesa/

    wget ftp://ftp.freedesktop.org/pub/mesa/$MESA_VERSION/mesa-$MESA_VERSION.tar.gz
    gunzip mesa-$MESA_VERSION.tar.gz
    tar --extract -f mesa-$MESA_VERSION.tar
    cd mesa-$MESA_VERSION/
    mv mesa-$MESA_VERSION/ mesa


Build the driver

    mesa$ ./autogen.sh
    # For the debug resource, use --enable-debug
    mesa$ ./configure --with-vulkan-drivers=intel
    mesa$ make


Tweak icd.json file and output dir (mesa/lib)

    mesa$ cp src/intel/vulkan/intel_icd.x86_64.json lib/
    # modify the pathname in the intel_icd.x86_64.json file to be ./libvulkan_intel.so
    mesa$ rm -rf lib/gallium  # We don't need this
    mesa$ rm lib/nouveau_vieux_dri.so lib/r200_dri.so lib/radeon_dri.so # We don't need these


Finally, use mesa/lib as the input directory to the create_and_upload script.