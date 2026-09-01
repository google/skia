FROM gcr.io/skia-public/jsfiddle-base@sha256:15a837f0b3721257760c2958db9c06e13caaf2aa1a98f6fff5eb9854a7216f2c

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]