# Docker

Docker files for building different Skia targets.

## skia-release

This image is used to build Skia at TOT with SwiftShader.

There is a continuous process that builds this docker image, but if you
need to manually push a verison, then run the following commands:

    docker build -t skia-release ./docker/skia-release/
    docker tag skia-release gcr.io/skia-public/skia-release:prod
    docker push gcr.io/skia-public/skia-release:prod

## skia-wasm-release

This image is used to build the Web Assembly (WASM) libraries of Skia
at TOT.

There is a continuous process that builds this docker image, but if you
need to manually push a verison, then run the following commands:

    docker build -t skia-wasm-release ./docker/skia-wasm-release/
    docker tag skia-wasm-release gcr.io/skia-public/skia-wasm-release:prod
    docker push gcr.io/skia-public/skia-wasm-release:prod


## skia-with-swift-shader-base

This image is used to build a local checkout of Skia with SwiftShader and run the built
executables without requiring the SwiftShader be installed on the host.

For example (see build-with-swift-shader-and-run for more info), to reproduce a
fuzzer bug in SwiftShader:

    # First, copy the test case into $SKIA_ROOT, say $SKIA_ROOT/skbug_1234
    build-with-swift-shader-and-run "out/with-swift-shader/fuzz -t filter_fuzz -b /skia/skbug_1234"

There is a continuous process that builds this docker image (which only really changes
if SwiftShader is updated), but if you need to manually push a version, then run the
following commands:

    docker build -t skia-with-swift-shader-base ./docker/skia-with-swift-shader-base/
    docker tag skia-with-swift-shader-base gcr.io/skia-public/skia-with-swift-shader-base:prod
    docker push gcr.io/skia-public/skia-with-swift-shader-base:prod

## cmake-release

This image is used to build Skia using CMake.

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries. To push:

    docker build -t cmake-release ./cmake-release/
    docker tag cmake-release gcr.io/skia-public/cmake-release:3.13.1_v2
    docker push gcr.io/skia-public/cmake-release:3.13.1_v2

For testing the image locally, the following flow can be helpful:

    docker build -t cmake-release ./cmake-release/
    # Run bash in it to poke around and make sure things are properly
    # installed and configured. Also useful to get version of CMake.
    docker run -it cmake-release /bin/bash
    # Compile Skia in a local checkout with the local image
    docker run -v $SKIA_ROOT:/SRC -v /tmp/output:/OUT cmake-release /SRC/infra/docker/cmake/build_skia.sh

## binary-size

This image is used to build code size tree-maps of Skia

It gets manually pushed anytime there's an update to the Dockerfile or relevant
installed libraries. To push:

    docker build -t binary-size ./binary-size/
    docker tag binary-size gcr.io/skia-public/binary-size:v1
    docker push gcr.io/skia-public/binary-size:v1

For testing the image locally, the following flow can be helpful:

    docker build -t binary-size ./binary-size/
    # Run bash in it to poke around and make sure things are properly
    # installed and configured.
    docker run -it binary-size /bin/sh
    # analyze exe "skottie_tool" in build directory out/Release
    docker run -v $SKIA_ROOT/out/Release:/IN -v /tmp/output:/OUT binary-size /opt/binary_size/src/run_binary_size_analysis.py --library /IN/skottie_tool --destdir /OUT

## skia-build-tools

This image contains all the tools needed to build Skia.

To push a new version run:

    make push-skia-build-tools