FROM gcr.io/skia-public/shaders-base@sha256:43533a3b1a035922b3f5399d2e0c7966bb2a894f9fd271cad953b95d670b1e89

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]