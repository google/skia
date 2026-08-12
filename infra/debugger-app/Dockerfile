FROM gcr.io/skia-public/debugger-app-base@sha256:80b143eb15c5cbc47a24d971a6b18ae31ac05336ffad630a21d7461f23c1779c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]