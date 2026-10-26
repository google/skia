FROM gcr.io/skia-public/skottie-base@sha256:add81ce1dbe7d1bf1344ee949d81cebd2f5959d8cbfd061178c8ce43453e6bd5

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]