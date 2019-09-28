# Dockerfile that bundles chromium's binary_size tools
# to build treemaps of code size for executables.
FROM alpine:latest as dart-sdk-checkout

RUN apk update && apk upgrade && \
    apk add git

WORKDIR /tmp/

RUN git clone --depth 1 https://github.com/dart-lang/sdk.git

#############################################################################
# Multi-stage build part 2, in which we only have the python runtime and
# and the scripts we need to analyze the binary.
#############################################################################

FROM alpine:latest as analyzer

RUN apk update && apk upgrade && \
    apk add python binutils

WORKDIR /opt

COPY --from=dart-sdk-checkout /tmp/sdk/runtime/third_party/ /opt/
