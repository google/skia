FROM gcr.io/skia-public/skottie-base@sha256:c8bbdc3f1219e16073d282b0a069e93ff66fa5e92179c64e95884ec3d958bcf4

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]