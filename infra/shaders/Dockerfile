FROM gcr.io/skia-public/shaders-base@sha256:5e22a60082ec679955d92ae83efd5286cdd087a50a5a395f7a7c8d011c00faa5

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]