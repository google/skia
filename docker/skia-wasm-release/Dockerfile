# Dockerfile for building the WASM libraries used by jsfiddle.skia.org
FROM gcr.io/skia-public/emsdk-release:prod

RUN cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git' \
  && mkdir -p /tmp/skia \
  && cd /tmp/skia \
  && /tmp/depot_tools/fetch skia

RUN cd /tmp/skia/skia \
  && python tools/git-sync-deps \
  && ./bin/fetch-gn

# PathKit should be in /tmp/skia/skia/out/pathkit/
RUN /tmp/skia/skia/modules/pathkit/compile.sh

# CanvasKit should be in /tmp/skia/skia/out/canvaskit_wasm
RUN /tmp/skia/skia/experimental/canvaskit/compile.sh
