FROM gcr.io/skia-public/shaders-base@sha256:8e3b6d97c1c6d7b94b67ceb06e6fb4e00cee99b3dc3de16ac8480035ff69e411

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]