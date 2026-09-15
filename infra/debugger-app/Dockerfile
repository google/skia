FROM gcr.io/skia-public/debugger-app-base@sha256:6b9f11c17f315dd6c61364be9dbf6f6cbb7b0d71ed14440c106e093925866550

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]