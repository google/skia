FROM gcr.io/skia-public/debugger-app-base@sha256:c708abbcb137efe7826778a775b8b0517754b8507643bcc79e7234f50ca98704

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]