FROM gcr.io/skia-public/shaders-base@sha256:61415213cc70429fee578a25a5d1f9a04d6e479269c1f94738c91368b8feaa83

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]