This directory contains the build rules to create the final Docker image for
the Skia jsfiddle hosted at jsfiddle.skia.org.

This build rule inserts the necessary Skia artifact (CanvasKit and PathKit) into
an intermediate Docker image created in the Skia infrastructure repository at
https://skia.googlesource.com/buildbot/+/refs/heads/main/jsfiddle/BUILD.bazel.
This final docker image is then uploaded to GCR and deployed to skia.org.

To manually build a local Docker image:

    make build

This can then be run locally by:

    docker run -p 8080:8000 -it <image ID>

or debugged by:

    docker run -it --entrypoint /bin/sh <image ID>

This docker image is automatically built and pushed to GCR by Louhi. If there
is a need to manually push it this can be done as so:

    make push_jsfiddle_I_am_really_sure