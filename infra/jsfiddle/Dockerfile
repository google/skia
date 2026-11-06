FROM gcr.io/skia-public/jsfiddle-base@sha256:36c8f5cd92b68fc34f46fba8a543a239fdcbc38f22ab8d997a177c9f03773fbf

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]