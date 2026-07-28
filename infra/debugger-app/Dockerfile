FROM gcr.io/skia-public/debugger-app-base@sha256:a46a65d7ba05745fb757c8ea8dde14d62f1cfca0f5687a642e05cf3bbbd7bb57

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]