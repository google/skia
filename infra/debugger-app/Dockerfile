FROM gcr.io/skia-public/debugger-app-base@sha256:189167a0ccc9d61ccb5c632a12eb0a00d9c5cb273305ee1e77d4a3ab17f8c55e

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]