FROM gcr.io/skia-public/debugger-app-base@sha256:14d0e3f817f2e28d6d30e45e6718a7285ef0b9c0dfe5a2e3019766f261089cb0

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]