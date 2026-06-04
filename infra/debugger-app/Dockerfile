FROM gcr.io/skia-public/debugger-app-base@sha256:8e5327247e0948fd96a86c5d4a63c7cd75b800582d84aea8f757a500730b9d34

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]