FROM gcr.io/skia-public/jsfiddle-base@sha256:2b12e98d373067abfb3343c1b3f8bd70ef4144d188aefefe1b8ff0afed5930c9

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]