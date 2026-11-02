FROM gcr.io/skia-public/shaders-base@sha256:9532e81b481a4fd1983a868c85cfb07a70391e7300f316b9e0d6220f89346f6e

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]