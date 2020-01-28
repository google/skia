# A Docker image that has the Emscripten SDK installed to /opt/emsdk
# Use this image to compile C/C++ code to WASM.
# It has the Java Runtime to allow the use of the Closure JS compiler/minifier.

FROM gcr.io/skia-public/skia-build-tools:latest
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  nodejs \
  default-jre

RUN cd /opt \
  && git clone https://github.com/emscripten-core/emsdk.git

WORKDIR /opt/emsdk

RUN ./emsdk update-tags

RUN ./emsdk install sdk-1.39.6-64bit

RUN ./emsdk activate sdk-1.39.6-64bit

RUN /bin/bash -c "source ./emsdk_env.sh"

ENV EMSDK=/opt/emsdk

RUN mkdir -p /OUT /SRC

