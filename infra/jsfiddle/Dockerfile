FROM gcr.io/skia-public/jsfiddle-base@sha256:a1fa0e2b6d9cff8adbb8fcdd944712c692b59999712ba588b0896ea3d42be6aa

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]