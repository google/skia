FROM gcr.io/skia-public/debugger-app-base@sha256:a9150fe5f17f3d9fcc6e0f0514ceb029d7dfd44509fd7a0897c697d8269ae9b8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]