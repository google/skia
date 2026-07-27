FROM gcr.io/skia-public/debugger-app-base@sha256:b4889b3141e302162714fc8d0fa664c264197bcd92875d7b230be34b4022eb04

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]