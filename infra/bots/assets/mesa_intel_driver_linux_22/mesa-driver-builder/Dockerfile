FROM debian:bookworm

ENV DEBIAN_FRONTEND noninteractive

# deb-src is not included by default in the docker image, so we duplicate every
# line in `sources.list` that begins with `deb` and create a new line that
# starts with `deb-src`.
RUN cp /etc/apt/sources.list /etc/apt/sources.list~ && \
    sed -i 's/\(^deb\)\(.*\)$/\1\2\n\1-src \2/' /etc/apt/sources.list

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y wget curl clang meson rsync jq && \
    apt-get build-dep -y mesa && \
    rm -rf /var/lib/apt/lists/*

COPY ./build_mesa.sh /opt/build_mesa.sh

# Add entry point so we can run the container w/o having to specifying the
# script name.
ENTRYPOINT ["/opt/build_mesa.sh"]
