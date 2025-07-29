FROM gcr.io/skia-public/jsfiddle-base@sha256:74f9e2cd8bc15e820442cbd348ed7ee19dea2c58a6c30a75f51d365c1f1a0921

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY pathkit.js /usr/local/share/jsfiddle/dist/pathkit.js
COPY pathkit.wasm /usr/local/share/jsfiddle/dist/pathkit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]