FROM gcr.io/skia-public/shaders-base@sha256:503b59e71e1e42869c0af0ce75dceaf3da5dbde5bf2d8268b9af3eafedfe5aa6

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]