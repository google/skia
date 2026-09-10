FROM gcr.io/skia-public/jsfiddle-base@sha256:b8b1c1cfc6a573e9b267a48cfcd5f542ca73a9da42b953604938677799771693

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]