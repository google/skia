FROM gcr.io/skia-public/skottie-base@sha256:867c02555f3749227d7f7f6f757b6b8a930cb2cd44ba8d151dfdda6a2c6797a9

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]