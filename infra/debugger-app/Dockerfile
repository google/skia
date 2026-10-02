FROM gcr.io/skia-public/debugger-app-base@sha256:cca7f4fd69fd45e009f5c917cc83409a6da9d9b5e180689e0e2e53813b564052

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]