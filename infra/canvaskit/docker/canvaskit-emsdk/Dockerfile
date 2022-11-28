# A Docker image that augments the Emscripten SDK Docker image
# with anything needed to build CanvasKit

FROM gcr.io/skia-public/emsdk-base:3.1.26_v2

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  libfreetype6-dev
