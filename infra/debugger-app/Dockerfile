FROM gcr.io/skia-public/debugger-app-base@sha256:4fab2c75015c18a2c25f30af5131a391b85e8a8b8f2894b9ca3d5aa2ea2d7506

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]