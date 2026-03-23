FROM gcr.io/skia-public/shaders-base@sha256:92fc9e89142cbc2f8b2b2e24c2ba0dfcba8204e216a394e1aa00969d046f889a

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]