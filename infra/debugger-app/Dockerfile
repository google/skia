FROM gcr.io/skia-public/debugger-app-base@sha256:eee99def8bd85a90c69222e8fed06a590d740cff2c8a48d41366f4ed874ac9cb

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]