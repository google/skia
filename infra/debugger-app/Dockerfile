FROM gcr.io/skia-public/debugger-app-base@sha256:3379d5a669e8639bac090df9b66eb436f9f6da66043f7925751d63c800d165eb

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]