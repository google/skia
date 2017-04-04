ARM (hard float) sysroot for cross-compiling c++ code on a x86_64 Linux bot.


    sudo apt-get install libstdc++-4.8-dev-armhf-cross libgcc-4.8-dev-armhf-cross binutils-arm-linux-gnueabihf

And then run this script to package up the installed results of the debian.
Take a peak at `/usr/arm-linux-gnueabihf/include/c++/4.8.X` - you may need to update the
include paths if that number changed from the previous release.