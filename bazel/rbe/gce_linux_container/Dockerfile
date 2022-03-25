# This image was created with the following 2 docker commands:
# FROM debian:bookworm-slim
# RUN apt-get update && \
#    apt-get install -y clang openjdk-11-jdk-headless
# Then, after seeing what the hash was of the base image and what versions of clang and the JDK
# were installed, those versions were pinned and the image was rebuilt and pushed to make sure
# those precise versions were used. This is to reach at least SLSA level 1 in that we know exactly
# what versions of the binaries are installed on the images we used to build things.
FROM debian@sha256:8c6a7e41209df74f51677a06b6944be803d662f821a18890d67eecf5dd2962e5
RUN apt-get update && \
    apt-get install -y clang=1:13.0-54 openjdk-11-jdk-headless=11.0.14+9-1
ENV JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"