FROM gcr.io/skia-public/debugger-app-base@sha256:5d79f6dc414664fededc905ac3d885973448761aafcc26ff402b55d93eccfc8a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]