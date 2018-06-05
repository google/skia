Docker
======

Docker files for building different Skia targets.

skia-release
------

This image is used to build Skia at TOT with SwiftShader.

There is a continuous process that builds this docker image, but if you
need to manually push a verison, then run the following commands:

    docker build -t skia-release ./docker/skia-release/
    docker tag skia-release gcr.io/skia-public/skia-release:prod
    docker push gcr.io/skia-public/skia-release:prod


skia-with-swift-shader-base
------

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