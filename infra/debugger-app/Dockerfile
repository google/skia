FROM gcr.io/skia-public/debugger-app-base@sha256:93ebcb86c3cf0d39ed602653bfdc19b2882de62605ba4e2ee82c6519229322b3

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]