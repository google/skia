FROM gcr.io/skia-public/debugger-app-base@sha256:0503e460e8f2b5f5034d9b32e10504809dcab4c3ec49a0ccf6133baac7b7762b

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]