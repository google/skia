FROM gcr.io/skia-public/debugger-app-base@sha256:7d52f7d9b6c72fc06cd0e348fd604489ae423ca8f8ef41a3bbfbfc1925eb6f61

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]