FROM gcr.io/skia-public/skottie-base@sha256:af6d4026789b64b65b0bbbc54a8f7a96a7657818566f1fc7a7814c371f40e749

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]