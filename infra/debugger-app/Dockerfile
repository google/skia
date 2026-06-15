FROM gcr.io/skia-public/debugger-app-base@sha256:ecbc8d84afd7573bf40352165d2e7bc8c57a28e354a299d3b73df22f2698668f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]