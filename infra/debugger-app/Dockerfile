FROM gcr.io/skia-public/debugger-app-base@sha256:c9d2193a29a15308e07a61320cf817005ff063e68dcaf03384a6c653bb916801

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]