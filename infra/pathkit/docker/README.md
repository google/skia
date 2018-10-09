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

    docker build -t emsdk-base ./emsdk-base/
    EMSDK_VERSION="1.38.6_jre"
    docker tag emsdk-base gcr.io/skia-public/emsdk-release:$EMSDK_VERSION
    docker push gcr.io/skia-public/emsdk-release:$EMSDK_VERSION

For testing the image locally, the following flow can be helpful:

    docker build -t emsdk-base ./emsdk-base/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it emsdk-release /bin/bash
    # Compile PathKit with the local image
    docker run -v $SKIA_ROOT:/SRC -v $SKIA_ROOT/out/dockerpathkit:/OUT emsdk-base /SRC/infra/pathkit/build_pathkit.sh

karma-chrome-tests
------------------

This image has Google Chrome and karma/jasmine installed on it, which can
be used to run JS tests.

This image is standalone and does not have any extra dependencies that make
it Skia-exclusive.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    docker build -t karma-chrome-tests ./karma-chrome-tests/
    # check the version of chrome with the following:
    docker run karma-chrome-tests /usr/bin/google-chrome-stable --version
    CHROME_VERSION="68.0.3440.106_v5"  # use v1, v2, etc for any re-spins of the container.
    docker tag karma-chrome-tests gcr.io/skia-public/karma-chrome-tests:$CHROME_VERSION
    docker push gcr.io/skia-public/karma-chrome-tests:$CHROME_VERSION

Of note, some versions (generally before Chrome 60) run out of space on /dev/shm when
using the default Docker settings.  To be safe, it is recommended to run the container
with the flag --shm-size=2gb.

For testing the image locally, the following can be helpful:

    docker build -t karma-chrome-tests ./karma-chrome-tests/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it --shm-size=2gb karma-chrome-tests /bin/bash
    # Run the tests (but not capturing Gold output) with the local source repo
    docker run --shm-size=2gb -v $SKIA_ROOT:/SRC karma-chrome-tests karma start /SRC/infra/pathkit/karma-docker.conf.js --single-run

gold-karma-chrome-tests
------------------

This image has Google Chrome and karma/jasmine installed on it, which can
be used to run JS tests.

This image assumes the runner wants to collect the output images and JSON data
specific to Skia Infra's Gold tool (image correctness).

It gets manually pushed anytime there's an update to the Dockerfile or the parent
image (karma-chrome-tests).

    # Run the following from $SKIA_ROOT/infra/pathkit
    make gold-docker-image
    # check the version of chrome with the following:
    docker run gold-karma-chrome-tests /usr/bin/google-chrome-stable --version
    CHROME_VERSION="68.0.3440.106_v4"  # use v1, v2, etc for any re-spins of the container.
    docker tag gold-karma-chrome-tests gcr.io/skia-public/gold-karma-chrome-tests:$CHROME_VERSION
    docker push gcr.io/skia-public/gold-karma-chrome-tests:$CHROME_VERSION

Of note, some versions (generally before Chrome 60) run out of space on /dev/shm when
using the default Docker settings.  To be safe, it is recommended to run the container
with the flag --shm-size=2gb.

For testing the image locally, the following can be helpful:

    # Run the following from $SKIA_ROOT/infra/pathkit
    make gold-docker-image
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it --shm-size=2gb gold-karma-chrome-tests /bin/bash
    # Run the tests and collect Gold output with the local source repo
    mkdir -p -m 0777 /tmp/dockergold
    docker run --shm-size=2gb -v $SKIA_ROOT:/SRC -v /tmp/dockergold:/OUT gold-karma-chrome-tests /SRC/infra/pathkit/test_pathkit.sh
