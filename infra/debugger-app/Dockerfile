FROM gcr.io/skia-public/debugger-app-base@sha256:a2de4c63b16c72eb67aea3342593dc7728d451a16d153444618c4d0b4bb5a461

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]