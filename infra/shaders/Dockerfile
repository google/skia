FROM gcr.io/skia-public/shaders-base@sha256:fc1915256a36ac2ca8be5734bd853bcadf31cf708ef519a5553429725b12d18e

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]