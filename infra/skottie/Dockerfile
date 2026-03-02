FROM gcr.io/skia-public/skottie-base@sha256:0c340feb28bdab117fd1132c3fd52f1d6038deea7833e40429f679ee6659fd1b

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]