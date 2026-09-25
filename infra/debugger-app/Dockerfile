FROM gcr.io/skia-public/debugger-app-base@sha256:b4346f3d4f5cc2d309fa5fbdcf37a322390ad7ce8a8c634d0251cdbdc2f12066

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]