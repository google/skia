FROM gcr.io/skia-public/skottie-base@sha256:e9e9bc8435a6bbc7e89f2d1074943c7bda6dc1e61c6beffcf22eb64aae1af15c

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]