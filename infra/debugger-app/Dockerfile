FROM gcr.io/skia-public/debugger-app-base@sha256:6450bc7cb052e4d8bf5bc89b5edc43543ce5fd2f1d2267dcdbd0609f6e40b12a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]