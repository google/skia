FROM gcr.io/skia-public/debugger-app-base@sha256:776a8611a0e0023de59d4a6940caf0077e0b8fe6584354d4ff8dfb1c16463e71

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]