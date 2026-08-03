FROM gcr.io/skia-public/shaders-base@sha256:9278a7788a7e56da9a9cdcff598e30a3df14622b7d3828f13b0c1306bebf2eeb

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]