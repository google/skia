FROM gcr.io/skia-public/skottie-base@sha256:6c01ff2ce17b19d4d033efcc15f9b31333d385998b9422011ed86f4aa62b17eb

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]