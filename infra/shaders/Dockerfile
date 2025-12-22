FROM gcr.io/skia-public/shaders-base@sha256:b80f1f54f7aaa5ff14314048da019c1924a74990dbc9aa5468ef320eabee3764

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]