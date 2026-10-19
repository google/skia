FROM gcr.io/skia-public/shaders-base@sha256:b5d16ac50a7b5b02133619bb77eb6b8dfbb66ce0f727ce8f0197be34140d7f67

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]