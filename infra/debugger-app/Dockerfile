FROM gcr.io/skia-public/debugger-app-base@sha256:85bac83c145edeb3beceef8002f0d2bfbd79fec95935f90607275761710a80a7

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]