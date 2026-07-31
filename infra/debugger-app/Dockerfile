FROM gcr.io/skia-public/debugger-app-base@sha256:2d7debe40c61e210af26d6cb6dde14f8a3935f7614bde4becb3c0abbf34b7699

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]