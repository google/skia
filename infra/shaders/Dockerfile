FROM gcr.io/skia-public/shaders-base@sha256:2c861cb826145a1bb347b48ae2eff4e88e1941abfe94fe4a256b660db661255f

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]