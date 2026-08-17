FROM gcr.io/skia-public/debugger-app-base@sha256:908aa7ccc3a9bfba1a8aae5706a595d6337fb2410f56e59c2e349bd25efe3040

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]