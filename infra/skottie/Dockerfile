FROM gcr.io/skia-public/skottie-base@sha256:125a9ff87309676e15747f0643b3f95b40a10d049e098bf1e7a88bb70cd43ec8

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]