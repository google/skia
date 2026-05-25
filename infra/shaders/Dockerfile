FROM gcr.io/skia-public/shaders-base@sha256:9f862803ee5b7ce2f6b5af6fd18a7426d56a14585ca2160132ae149af809a04a

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]