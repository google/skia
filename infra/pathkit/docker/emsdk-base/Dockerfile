# A Docker image that has the Emscripten SDK installed to /opt/emsdk
# Use this image to compile C/C++ code to WASM.

FROM launcher.gcr.io/google/clang-debian9
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  nodejs \
  default-jre

RUN cd /opt \
  && git clone https://github.com/juj/emsdk.git

WORKDIR /opt/emsdk

RUN ./emsdk update-tags

# These versions were available and worked on my local desktop as of Nov 6 2018.
RUN ./emsdk install sdk-1.38.27-64bit

RUN ./emsdk activate sdk-1.38.27-64bit

RUN /bin/bash -c "source ./emsdk_env.sh"

ENV EMSDK=/opt/emsdk

RUN mkdir -p /OUT /SRC

