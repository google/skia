FROM gcr.io/skia-public/debugger-app-base@sha256:4ab9cd4f66b2d6dfc80fcb9f8780eb40c9343ec89b3222ebd756d3e527c9a3c0

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]