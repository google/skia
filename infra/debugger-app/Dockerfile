FROM gcr.io/skia-public/debugger-app-base@sha256:3824c8346c3883ae92c2d692658425222f98efb65614b805fc0e5cf7a014cb4c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]