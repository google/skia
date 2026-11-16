FROM gcr.io/skia-public/shaders-base@sha256:d0d2988cc5c7efc2dc25bedaba1b2a611e63bf0022cca1f256c0d24bba69b231

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]