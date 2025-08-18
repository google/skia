FROM gcr.io/skia-public/jsfiddle-base@sha256:c4343b58423eee4b68a53d1a5e075aecedc3a6958fb0322d3a98ee9c90e9e733

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY pathkit.js /usr/local/share/jsfiddle/dist/pathkit.js
COPY pathkit.wasm /usr/local/share/jsfiddle/dist/pathkit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]