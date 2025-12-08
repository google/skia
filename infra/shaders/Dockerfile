FROM gcr.io/skia-public/shaders-base@sha256:c5ea487bec787083dc0b95499d421c065aa973fa7435dda116e2e9690f462eb7

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]