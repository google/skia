# Creating the Mesa Intel Linux driver asset (which supports Vulkan)

Using the automated asset python scripts requires that Docker be installed.

    $ sk asset upload mesa_intel_driver_linux_22

See below for steps on just building the driver.

## WARNING

This asset is `mesa_intel_driver_linux_22`.

There is also `mesa_intel_driver_linux`,  which contains pre-version
22 copies of the Mesa drivers, that's because in v22 Mesa dropped support for
"legacy" drivers which covers many of our existing Intel jobs.

## Using Docker

It is easiest to just use the Dockerfile provided:

    docker build -t mesa-driver-builder:latest ./mesa-driver-builder/
    docker run --volume /tmp/out:/OUT --env MESA_VERSION=22.1.3 mesa-driver-builder:latest

You may change `/tmp/out` to be the desired output directory and `22.1.3` to be the desired
version of the mesa driver.

Finally, use `/tmp/out` as the input directory to the upload script.

## Testing

Testing both the Vulkan and GLX drivers is done via setting environment variables.

For example, if the above is run and the build drivers are in `/tmp/out/` then they can be tested via:

    cd /tmp/out
    LIBGL_DRIVERS_PATH=`pwd` glxinfo | grep -i Mesa
    VK_ICD_FILENAMES="./intel_icd.x86_64.json" vulkaninfo | grep Mesa

N.B. Make sure to pick a version of Mesa that your desktop is not currently
using and then run both commands above both with and w/o the environment
variables set to confirm that the drivers are really being picked up. For example:

With the 22.1.3 drivers:

~~~console
chrome-bot@skia-e-linux-600:~/mesa/foo$ VK_ICD_FILENAMES="./intel_icd.x86_64.json" vulkaninfo | grep Mesa
VK_LAYER_MESA_overlay (Mesa Overlay layer) Vulkan version 1.3.211, layer version 1:
        driverName      = Intel open-source Mesa driver
        driverInfo      = Mesa 22.1.3
        driverName                                           = Intel open-source Mesa driver
        driverInfo                                           = Mesa 22.1.3
~~~

Compared to the installed 22.0.5 drivers:

~~~console
chrome-bot@skia-e-linux-600:~/mesa/foo$  vulkaninfo | grep Mesa
WARNING: lavapipe is not a conformant vulkan implementation, testing use only.
VK_LAYER_MESA_overlay (Mesa Overlay layer) Vulkan version 1.3.211, layer version 1:
WARNING: lavapipe is not a conformant vulkan implementation, testing use only.
        driverName      = Intel open-source Mesa driver
        driverInfo      = Mesa 22.0.5
        driverName                                           = Intel open-source Mesa driver
        driverInfo                                           = Mesa 22.0.5
        driverInfo      = Mesa 22.0.5 (LLVM 14.0.4)
        driverInfo                                           = Mesa 22.0.5 (LLVM 14.0.4)
~~~