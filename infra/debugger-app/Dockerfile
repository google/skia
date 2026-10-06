FROM gcr.io/skia-public/debugger-app-base@sha256:c34ed9a58f2f1b57c9002e3604b4b6ef6f9e67aa02dd581c7c73784892643bba

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]