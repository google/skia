FROM gcr.io/skia-public/shaders-base@sha256:0b85ebb8cd88d4a56a23b54753a75df475f0380e16fd190cf7754fea0f4e0cef

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]