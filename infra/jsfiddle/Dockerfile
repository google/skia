FROM gcr.io/skia-public/jsfiddle-base@sha256:d568314f30143fef08250bb89d680bee4c60cba66de40386f1005d10fb708e79

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]