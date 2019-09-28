# A Docker image that augments the Emscripten SDK Docker image
# with anything needed to build Canvaskit

FROM gcr.io/skia-public/emsdk-release:1.38.27_v1

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  libfreetype6-dev