FROM gcr.io/skia-public/jsfiddle-base@sha256:3c8958265e8421d7a7fb79b3114ea35b19a0830e0c8c971d80e70a318faead80

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]