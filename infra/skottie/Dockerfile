FROM gcr.io/skia-public/skottie-base@sha256:d3e0a508e29cada6250d890e0f4b0e516167ec8d042255b88df03a331c3efc59

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]