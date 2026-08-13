FROM gcr.io/skia-public/debugger-app-base@sha256:ff9fa25f5892b5c7dab9fb829a8a4f354f71c39b71735b4b673a2e363b7c4918

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]