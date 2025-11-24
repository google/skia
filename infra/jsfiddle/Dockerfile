FROM gcr.io/skia-public/jsfiddle-base@sha256:ef6c6a80c82e8f42a4b8850ed1daa575599d0bed1bf36a9fb0c49b68c7c2363e

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]