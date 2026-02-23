FROM gcr.io/skia-public/shaders-base@sha256:40c834a730e358833b2989f111b510f2d2d31c8735a008a5616e3b0ff49b4b2c

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]