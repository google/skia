FROM gcr.io/skia-public/skottie-base@sha256:951c83390400234891497e09a3096cc88e9b0f7023fc3a54091f01e6a0174b22

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]