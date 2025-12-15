FROM gcr.io/skia-public/debugger-app-base@sha256:16db6a2be6c7ad613a02d2ffcc0ad3940d715e1a426e2e026e570e1952e0bfef

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]