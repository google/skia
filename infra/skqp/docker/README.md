Docker
======

Docker files to ease working with the Android SDK/NDK and SKQP.

android-skqp
-------------

This image has an Android emulator, the Android SDK and NDK installed on it.
Additionally, it has the dependencies needed to build SKQP - Clang, python, ninja.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries.

    # This will pause after the "Accept? (y/N)" prompt as it installs the NDK;
    # you do not need to hit "y".
    docker build -t android-skqp ./android-skqp/
    # use v2, v3 for respins; see http://gcr.io/skia-public/android-skqp for
    # latest version
    ANDROID_SDK_VERSION="8.1_v1"
    docker tag android-skqp gcr.io/skia-public/android-skqp:$ANDROID_SDK_VERSION
    docker push gcr.io/skia-public/android-skqp:$ANDROID_SDK_VERSION


For testing the image locally, the following flow can be helpful:

    docker build -t android-skqp ./android-skqp/

    # start an emulator
    docker run --privileged -d --name android_em \
        -e DEVICE="Samsung Galaxy S6" \
        -v $SKIA_ROOT:/SRC \
        -v $SKIA_ROOT/out/skqp:/OUT \
        android-skqp

    # attach to that emulator
    docker exec -it android_em /bin/bash

    # Compile SKQP
    docker run -it --rm -w /SRC/infra/skqp \
        -v $SKIA_ROOT:/SRC \
        android-skqp ./build_apk.sh

    # Run SKQP (can't mount anything with -v here, must do it on
    # original docker run)
    docker exec -it android_em /SRC/infra/skqp/run_skqp.sh

    # Cleanup
    docker kill android_em
    docker rm android_em
