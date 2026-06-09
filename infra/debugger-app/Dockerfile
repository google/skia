FROM gcr.io/skia-public/debugger-app-base@sha256:d2178d1c149d76b659dac5773da46fbf3db413fcd3ed47781bf978e77c1a0b99

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]