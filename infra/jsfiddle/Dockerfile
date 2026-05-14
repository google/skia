FROM gcr.io/skia-public/jsfiddle-base@sha256:508fbad5adbd5fcd80bebe3a5b5796420c9a1f723186c15cb1bc5ca50d4082e5

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]