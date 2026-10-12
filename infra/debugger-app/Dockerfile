FROM gcr.io/skia-public/debugger-app-base@sha256:3cea9093547886c98776a3072bd9ab79d2d295bd402208402959fb03d8575707

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]