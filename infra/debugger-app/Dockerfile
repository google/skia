FROM gcr.io/skia-public/debugger-app-base@sha256:a0d5ffb35057cd1e647e65a25664ea93a96bce8c1cda8bc3fa4a0c2626df823d

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]