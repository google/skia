FROM gcr.io/skia-public/debugger-app-base@sha256:2e9f350ffc36c14be24970dd1bb7ca06f018ed7b7292a8ee8e2face1cc92e3cb

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]