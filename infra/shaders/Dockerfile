FROM gcr.io/skia-public/shaders-base@sha256:a05e895e8ba2d66be83c8ea2a99a82127dccede2b37a43a4975ca10d27000085

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]