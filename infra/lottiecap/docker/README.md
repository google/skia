Docker
======

Docker files to handle Gold + lottie-web integration


lottie-web-puppeteer
--------------------

This image has Google Chrome, [puppeteer](https://github.com/GoogleChrome/puppeteer),
and a few other tools for automating web-browser tests.

This image is standalone and does not have any extra dependencies that make
it Skia-exclusive.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    docker build -t lottie-web-puppeteer ./lottie-web-puppeteer/
    LOTTIE_VERSION="v2"  # use v1, v2, etc for any re-spins of the container.
    docker tag lottie-web-puppeteer gcr.io/skia-public/lottie-web-puppeteer:$LOTTIE_VERSION
    docker push gcr.io/skia-public/lottie-web-puppeteer:$LOTTIE_VERSION

Of note, some versions (generally before Chrome 60) run out of space on /dev/shm when
using the default Docker settings.  To be safe, it is recommended to run the container
with the flag --shm-size=2gb.

For testing the image locally, the following can be helpful:

    docker build -t lottie-web-puppeteer ./lottie-web-puppeteer/
    # Run bash in it to poke around and make sure things are properly installed
    docker run -it --shm-size=2gb lottie-web-puppeteer /bin/bash
    # Create a screenshot of a single .json file which will be put in
    # $SKIA_ROOT/tools/lottiecap/docker_strip.png
    docker run -it -v $SKIA_ROOT:/SRC -v ~/lottie-samples:/LOTTIE_FILES -v $LOTTIE_ROOT/build/player:/LOTTIE_BUILD -w /SRC/tools/lottiecap lottie-web-puppeteer node /SRC/tools/lottiecap/lottiecap.js --input /LOTTIE_FILES/body_movin.json --lottie_player /LOTTIE_BUILD/lottie.min.js --in_docker --output docker_strip.png

gold-lottie-web-puppeteer
-------------------------

This image has Google Chrome, [puppeteer](https://github.com/GoogleChrome/puppeteer),
and a few other tools for automating web-browser tests.

This image assumes the runner wants to collect the output images and JSON data
specific to Skia Infra's Gold tool (image correctness).

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    # Run the following from $SKIA_ROOT/infra/pathkit
    make gold-docker-image
    LOTTIE_VERSION="v2"  # use v1, v2, etc for any re-spins of the container.
    docker tag gold-lottie-web-puppeteer gcr.io/skia-public/gold-lottie-web-puppeteer:$LOTTIE_VERSION
    docker push gcr.io/skia-public/gold-lottie-web-puppeteer:$LOTTIE_VERSION


Of note, some versions (generally before Chrome 60) run out of space on /dev/shm when
using the default Docker settings.  To be safe, it is recommended to run the container
with the flag --shm-size=2gb.

For testing the image locally, the following can be helpful:

    # Run the following from $SKIA_ROOT/infra/pathkit
    make gold-docker-image
    docker run -it --shm-size=2gb gold-lottie-web-puppeteer /bin/bash
    # Collect the gold output with the local source repo and *all* of the files
    # from lottie-samples
    mkdir -p -m 0777 /tmp/dockerout
    docker run -v ~/lottie-samples:/LOTTIE_FILES -v $SKIA_ROOT:/SRC -v $LOTTIE_ROOT/build/player:/LOTTIE_BUILD -v /tmp/dockerout:/OUT gold-lottie-web-puppeteer /SRC/infra/lottiecap/docker/lottiecap_gold.sh