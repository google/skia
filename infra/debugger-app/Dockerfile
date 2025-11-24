FROM gcr.io/skia-public/debugger-app-base@sha256:9c01087f9cc73d6703e6bb139414baf31d65f3570e6898a2defdca961cbb6964

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]