FROM gcr.io/skia-public/skottie-base@sha256:a31fac6e3da4c2d60de5a9080e88f8e7d18810fbae703c199d3c019bc78b000b

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]