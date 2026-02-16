FROM gcr.io/skia-public/skottie-base@sha256:4dc2907743fd0802b19de3301873391dd4709b22e592c42384d6a6e846cc0f8e

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]