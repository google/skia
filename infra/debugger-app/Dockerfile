FROM gcr.io/skia-public/debugger-app-base@sha256:edf7d9a2f883bae3fed1c4eab8ddbc9d5e350dd016a5b725ca0d234a286e9345

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]