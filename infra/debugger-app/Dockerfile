FROM gcr.io/skia-public/debugger-app-base@sha256:07fe585b169d83dab6316ad2d25e838708940ed6f6f6d01c09942dfa897a0154

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]