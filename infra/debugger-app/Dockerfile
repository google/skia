FROM gcr.io/skia-public/debugger-app-base@sha256:2ab63f6bb595aed68e4db22a282ad869d48043db2a25f4541e3e3e43cd775a71

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]