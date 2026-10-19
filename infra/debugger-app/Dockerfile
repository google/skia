FROM gcr.io/skia-public/debugger-app-base@sha256:21501cfbf4757db0ef0f2e82f59aae37228c97e9dcbd5c6fc28c010d12fd35fb

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]