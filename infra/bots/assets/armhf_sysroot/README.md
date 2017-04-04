ARM (hard float) sysroot for cross-compiling c++ code on a x86_64 Linux bot.

Run create_and_upload which installs the following debian packages and turns them
into a toolchain:

    libstdc++-4.8-dev-armhf-cross libgcc-4.8-dev-armhf-cross binutils-arm-linux-gnueabihf

Take a peak at `/usr/arm-linux-gnueabihf/include/c++/4.8.X` - you may need to update the
include paths if that number changed from the previous release (currently 4.8.4).