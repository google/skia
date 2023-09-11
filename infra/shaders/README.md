This directory contains the build rules to create the final Docker image for
the Skia shaders hosted at shaders.skia.org.

This build rule inserts the necessary Skia artifact (CanvasKit) into
an intermediate Docker image created in the Skia infrastructure repository at
https://skia.googlesource.com/buildbot/+/refs/heads/main/shaders/BUILD.bazel.
This final docker image is then uploaded to GCR and deployed to skia.org.

# Running locally

To manually build a local Docker image:

    make build

This can then be run locally by:

    docker run -p 8080:8000 -it <image ID>

or debugged by:

    docker run -it --entrypoint /bin/sh <image ID>

## Deployment

This docker image is automatically built and pushed to GCR by Louhi. If there
is a need to manually push it this can be done as so:

    make push_shaders_I_am_really_sure
