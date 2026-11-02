FROM gcr.io/skia-public/debugger-app-base@sha256:b94836982850792b73b0ea844ee507cf7afed0a2a1c69f484702da379b7652fa

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]