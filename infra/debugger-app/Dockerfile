FROM gcr.io/skia-public/debugger-app-base@sha256:22941fd05bf8f1bae2140d2c153f1c30cafb012a1771c937ef6a5e620c2d8417

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]