FROM gcr.io/skia-public/debugger-app-base@sha256:c74d260547707e09b8504887c2b7a7b44c29744f3b790506a9b6325c4efa36ec

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]