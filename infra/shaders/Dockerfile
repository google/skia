FROM gcr.io/skia-public/shaders-base@sha256:a9187546c2d11b4b1889ef64ab9b118ced002494470c9d9f44af4b52b7da1bd8

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]