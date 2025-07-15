FROM gcr.io/skia-public/skottie-base@sha256:6f141efaf8cbca6206e5e879f72217a9c7c2eb277a78946511ccf936c40369ed

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]