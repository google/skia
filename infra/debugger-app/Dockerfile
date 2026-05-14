FROM gcr.io/skia-public/debugger-app-base@sha256:518c0106b064705eb736886d8b1ac558fbf0aafdb36d06d5ff3883b0fb5726c8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]