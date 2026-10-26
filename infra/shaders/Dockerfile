FROM gcr.io/skia-public/shaders-base@sha256:70b316187d0f0cb9cad1e2975f65a3b1543f509b2e8514d8d1e037dad6a0c2c3

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]