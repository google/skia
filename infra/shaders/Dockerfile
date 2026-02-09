FROM gcr.io/skia-public/shaders-base@sha256:dd6a75180b0bd2f8f2788478f3713daf75f20ae5a45d529b15da8a7520f9a291

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]