FROM gcr.io/skia-public/skottie-base@sha256:af8d227d9756b1ba2ded6b7bc40f47b7e64ed93ff78b38e40cef1b8d5ecf94d9

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]