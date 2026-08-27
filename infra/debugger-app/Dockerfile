FROM gcr.io/skia-public/debugger-app-base@sha256:f7c6f5583429c2ead62fb9bea1c5195c9483b28d2b8fb0145e614a13e60cd11a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]