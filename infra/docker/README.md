# Docker

Docker files for building different Skia targets.

## cmake-release

This image is used to build Skia using CMake.

It should get rebuilt via Louhi (manual invocation) anytime there's an update to the Dockerfile
or relevant installed libraries.

For testing the image locally, the following flow can be helpful:

    docker build -t cmake-release ./cmake-release/
    # Run bash in it to poke around and make sure things are properly
    # installed and configured. Also useful to get version of CMake.
    docker run -it cmake-release /bin/bash
    # Compile Skia in a local checkout with the local image
    docker run -v $SKIA_ROOT:/SRC -v /tmp/output:/OUT cmake-release /SRC/infra/cmake/build_skia.sh

## binary-size

This image is used to build code size tree-maps of Skia

It should get rebuilt via Louhi (manual invocation) anytime there's an update to the Dockerfile
or relevant installed libraries.

For testing the image locally, the following flow can be helpful:

    docker build -t binary-size ./binary-size/
    # Run bash in it to poke around and make sure things are properly
    # installed and configured.
    docker run -it binary-size /bin/sh
    # analyze exe "skottie_tool" in build directory out/Release
    docker run -v $SKIA_ROOT/out/Release:/IN -v /tmp/output:/OUT binary-size /opt/binary_size/src/run_binary_size_analysis.py --library /IN/skottie_tool --destdir /OUT
