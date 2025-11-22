FROM gcr.io/skia-public/skottie-base@sha256:0b6ddddfa5b524e645c8010ecc53986107e7fd5ca62f3afc2d77368cf9ccbf5b

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]