FROM gcr.io/skia-public/jsfiddle-base@sha256:1b7f09f5d7a2ba459bc594c7639bfef1e4380ac91494905b9f61a2d88322b8ba

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]