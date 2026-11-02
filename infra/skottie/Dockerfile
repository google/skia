FROM gcr.io/skia-public/skottie-base@sha256:6fd7ae5562d0c07f8677a860b9270f67944fe83e1a1c3260865664480ad4d833

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]