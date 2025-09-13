This directory contains the build rules to create the final Docker image for
the Skia Skottie hosted at skottie.skia.org.

This build rule inserts the necessary Skia artifact (CanvasKit) into
an intermediate Docker image created in the Skia infrastructure repository at
https://skia.googlesource.com/buildbot/+/refs/heads/main/skottie/BUILD.bazel.
This final docker image is then uploaded to Artifact Registry and deployed to skia.org using Louhi.