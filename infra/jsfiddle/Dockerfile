FROM gcr.io/skia-public/jsfiddle-base@sha256:e97ab14ddb65854dc4273b54d9fada9e27f5d91e67924807aebb8f113be1c7c5

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]