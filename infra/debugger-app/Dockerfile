FROM gcr.io/skia-public/debugger-app-base@sha256:9c5025e806afaa02f19d520db92bbd989534025e1c6b1cd7aaa4f9a12e01c982

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]