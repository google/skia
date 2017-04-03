ARM (hard float) sysroot for cross-compiling c++ code.


    sudo apt-get install libstdc++-4.8-dev-armhf-cross

And then run this script to package up the installed results of the debian.
Take a peak at `/usr/arm-linux-gnueabihf/include/c++/4.8.X` - you may need to update the include paths
if that number changed from the previous release.