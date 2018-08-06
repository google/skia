Docker
======

Docker files for setting up a emscripten SDK environment that can be used for
compiling projects (e.g. Skia's PathKit) to WASM/asm.js.

emsdk-base
----------

This image is standalone and does not have any extra dependencies that make
it Skia-exclusive.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    docker build -t emsdk-base ./docker/emsdk-base/
    EMSDK_VERSION="1.38.6"
    docker tag emsdk-base gcr.io/skia-public/emsdk-release:$EMSDK_VERSION
    docker push gcr.io/skia-public/emsdk-release:$EMSDK_VERSION

For testing the image locally, the following flow can be helpful:

    docker build -t emsdk-base ./docker/emsdk-base/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it emsdk-release /bin/bash
    # Compile PathKit with the local image
    docker run -v $SKIA_ROOT:/SRC -v $SKIA_ROOT/out/dockerpathkit:/OUT emsdk-base /SRC/experimental/pathkit/docker/build_pathkit.sh

