FROM gcr.io/skia-public/skottie-base@sha256:0517426ca15b665ff23339390633d033ad9bbdc15029e2020b3e6da262f8e1a1

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]