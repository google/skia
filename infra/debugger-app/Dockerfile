FROM gcr.io/skia-public/debugger-app-base@sha256:acdd3c9e9e9fef81c9bd6f051f7a6e1b255e7c7dd73b7a1841afe7dcf905c307

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]