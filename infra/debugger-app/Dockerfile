FROM gcr.io/skia-public/debugger-app-base@sha256:fd26511d445aa6fb89ff975403ddff4ed5cda45b7075409d2c9c61533efc1200

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]