# This image was created with the following 2 docker commands:
#FROM debian:bullseye-slim
#RUN apt-get update && \
#    apt-get install -y clang openjdk-11-jdk-headless \
#        fonts-dejavu-extra fonts-noto-core fonts-noto-cjk \
#        fonts-noto-color-emoji
#
# Then, after seeing what the hash was of the base image and what versions of clang and the JDK
# were installed, those versions were pinned and the image was rebuilt and pushed to make sure
# those precise versions were used. This is to reach at least SLSA level 1 in that we know exactly
# what versions of the binaries are installed on the images we used to build things.
#
# Base Image Notes:
#   I would like to be on debian:bookworm, but ran into
#   https://medium.com/nttlabs/ubuntu-21-10-and-fedora-35-do-not-work-on-docker-20-10-9-1cd439d9921
#   so had to go back to bullseye.
#
# Package Notes:
#   clang, openjdk: Bazel needs a default C++ compiler and Java runtime, but this will not be used
#                   to build Skia code.
#   fonts-dejavu-extra: The default fonts-dejavu-core do not have an italic font
#   fonts-noto-*: Coverage for many gms that test fonts
#   zip: Needed by Bazel to generate a ZIP file under //bazel-testlogs with any undeclared test
#        outputs (used e.g. to recover images produced by GM tests).
FROM debian@sha256:b46fc4e6813f6cbd9f3f6322c72ab974cc0e75a72ca02730a8861e98999875c7

# Please keep each package in a separate line to facilitate bulk-removing the pinned versions (e.g.
# with multiple cursors in VSCode or Sublime Text) as instructed in the README.md file.
RUN apt-get update && \
    apt-get install -y \
        clang=1:11.0-51+nmu5 \
        openjdk-11-jdk-headless=11.0.18+10-1~deb11u1 \
        fonts-dejavu-extra=2.37-2 \
        fonts-noto-core=20201225-1 \
        fonts-noto-cjk=1:20201206-cjk+repack1-1 \
        fonts-noto-color-emoji=0~20200916-1 \
        zip=3.0-12

ENV JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
