FROM gcr.io/skia-public/debugger-app-base@sha256:6626544d3066339a90468064e4d148b7a9e1c30810c35a45a4aa59242034ef06

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]