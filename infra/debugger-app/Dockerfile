FROM gcr.io/skia-public/debugger-app-base@sha256:cd38a27b85c7d60103c3064a5e63f00aecde87120eb3a97d412ba63e305dde27

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]