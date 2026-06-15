FROM gcr.io/skia-public/skottie-base@sha256:870493f761669c6fe4d1d2e895a19463fe05d37457876fdf9fa94d1f06d51661

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]