FROM gcr.io/skia-public/jsfiddle-base@sha256:e7c1ac2e1c20e51c2366bf20aba781d717cd7eff6c07461909a4fb4c0002caaa

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]