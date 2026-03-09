FROM gcr.io/skia-public/skottie-base@sha256:59099cc38eafa7b82ff8a37aff2a2e7b24a9416af32b565e8f5dc202d0457da4

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]