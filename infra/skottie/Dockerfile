FROM gcr.io/skia-public/skottie-base@sha256:4be913bfbea590450cfc98f697195a6c51fba4bf353626ed66f927ea6f0281b2

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]