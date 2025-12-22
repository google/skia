FROM gcr.io/skia-public/skottie-base@sha256:292d73c2f82f078dadf5d703aa4234a6fb74ae36381985aee0dd56fe0914b9aa

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]