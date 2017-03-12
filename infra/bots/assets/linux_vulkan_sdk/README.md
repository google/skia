To create the vulkan sdk asset:

Install the vulkan sdk from https://vulkan.lunarg.com/signin on a linux machine

The default install dir is in the same directory as the .run file is (e.g. ~/Downloads).
Call the install directory $VULKAN_SDK.

When uploading the CIPD asset, use -s $VULKAN_SDK/VERSION/x86_64
