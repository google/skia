FROM gcr.io/skia-public/skottie-base@sha256:a61634294908a678d894f4cc7cb699154e462761599eb5c319caab8efecc3315

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]