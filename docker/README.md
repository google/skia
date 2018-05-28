Docker
======

Docker files for building different Skia targets.

Manual
------

There is a continuous process that builds this docker image, but if you
need to manually push a verison, then run the following commands:

    docker build -t skia-release ./docker/skia-release/
    docker tag skia-release gcr.io/skia-public/skia-release:prod
    docker push gcr.io/skia-public/skia-release:prod
