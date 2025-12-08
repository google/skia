FROM gcr.io/skia-public/debugger-app-base@sha256:4b94f92c03e7576f84018b3dfe5cdeef27e2af7209013d0c85dfef649bb9738d

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]