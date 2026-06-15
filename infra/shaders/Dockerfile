FROM gcr.io/skia-public/shaders-base@sha256:2ca02a2612d9ec6ed1e4948acedee546e93dc1654930800c304451acf66a709f

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]