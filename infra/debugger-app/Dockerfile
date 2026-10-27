FROM gcr.io/skia-public/debugger-app-base@sha256:d7095a0929717c4f801ce54bb9ecc6ead42698009268422d699e6d2554b60ae4

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]