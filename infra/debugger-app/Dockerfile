FROM gcr.io/skia-public/debugger-app-base@sha256:44807f17a4cc92ae26f81b18ea40bb6e6bf9c7a90d893d6f110028c01971148a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]