FROM gcr.io/skia-public/jsfiddle-base@sha256:7331278a35937390cf8b8781b0a23c57c120d2ea5e61e4233a65ec1fb3d48d3c

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]