FROM gcr.io/skia-public/debugger-app-base@sha256:773cb2ff3ea5abe1521e9184589ad662730b77ae0c1d1ab49d2c44aae3f4d166

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]