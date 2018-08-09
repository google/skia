Docker
======

Docker files to ease working with PathKit and WASM.

emsdk-base
----------

This image has an Emscripten SDK environment that can be used for
compiling projects (e.g. Skia's PathKit) to WASM/asm.js.

This image is standalone and does not have any extra dependencies that make
it Skia-exclusive.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    docker build -t emsdk-base ./docker/emsdk-base/
    EMSDK_VERSION="1.38.6_jre"
    docker tag emsdk-base gcr.io/skia-public/emsdk-release:$EMSDK_VERSION
    docker push gcr.io/skia-public/emsdk-release:$EMSDK_VERSION

For testing the image locally, the following flow can be helpful:

    docker build -t emsdk-base ./docker/emsdk-base/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it emsdk-release /bin/bash
    # Compile PathKit with the local image
    docker run -v $SKIA_ROOT:/SRC -v $SKIA_ROOT/out/dockerpathkit:/OUT emsdk-base /SRC/experimental/pathkit/docker/build_pathkit.sh

karma-chrome-tests
------------------

This image has Google Chrome and karma/jasmine installed on it, which can
be used to run JS tests.

This image is standalone and does not have any extra dependencies that make
it Skia-exclusive.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    docker build -t karma-chrome-tests ./docker/karma-chrome-tests/
    # check the version of chrome with the following:
    docker run karma-chrome-tests /usr/bin/google-chrome-stable --version
    CHROME_VERSION="68.0.3440.106_v1"  # use v1, v2, etc for any re-spins of the container.
    docker tag karma-chrome-tests gcr.io/skia-public/karma-chrome-tests:$CHROME_VERSION
    docker push gcr.io/skia-public/karma-chrome-tests:$CHROME_VERSION

Of note, some versions (generally before Chrome 60) run out of space on /dev/shm when
using the default Docker settings.  To be safe, it is recommended to run the container
with the flag --shm-size=2gb.

For testing the image locally, the following can be helpful:

    docker build -t karma-chrome-tests ./docker/karma-chrome-tests/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it --shm-size=2gb karma-chrome-tests /bin/bash
    # Run the tests with the local source repo
    docker run --shm-size=2gb -v $SKIA_ROOT:/SRC karma-chrome-tests karma start /SRC/experimental/pathkit/karma-docker.conf.js --single-run

