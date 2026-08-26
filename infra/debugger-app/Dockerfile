FROM gcr.io/skia-public/debugger-app-base@sha256:eb2e5e9721af47de0cc413c37e2514e892efc4d19b3810a21c5bc3ac9f315fa8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]