FROM gcr.io/skia-public/shaders-base@sha256:ea17f563d60c53d175b381b751e58d2f5ec846c88d9ba0a9c694cd553e9725b1

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]