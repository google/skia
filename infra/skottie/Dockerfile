FROM gcr.io/skia-public/skottie-base@sha256:26e8a34330594f5aedbdbd24a21ed893d63745791b5c5de294e644946f558c7b

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]