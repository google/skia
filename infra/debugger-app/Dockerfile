FROM gcr.io/skia-public/debugger-app-base@sha256:e8f6bad3ec15cf07f7d1218ca47c45f068371fcb4a4b1645023267c8b3875985

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]