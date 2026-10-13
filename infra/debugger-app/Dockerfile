FROM gcr.io/skia-public/debugger-app-base@sha256:b3e2e2aaaabb44a7907d77e40785487d511f103b3b0524d0475dbdc8650e120c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]