FROM gcr.io/skia-public/debugger-app-base@sha256:51db08072579415f922716b1eb228f060cbe841711d64f23c5ee42ee3f8b4d73

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]