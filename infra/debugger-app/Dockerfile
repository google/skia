FROM gcr.io/skia-public/debugger-app-base@sha256:367a8e8a42947176e4ca34794a2f1f224a3a9b384b554c107ac09d408c46e1d6

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]