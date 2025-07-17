FROM gcr.io/skia-public/jsfiddle-base@sha256:85c7d3d4b62f001f7a08acff56c68bb06a551037839adf40aa70853e62f9133d

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY pathkit.js /usr/local/share/jsfiddle/dist/pathkit.js
COPY pathkit.wasm /usr/local/share/jsfiddle/dist/pathkit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]