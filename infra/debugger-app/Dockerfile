FROM gcr.io/skia-public/debugger-app-base@sha256:bc58f4cf14cec65915355e01cd7a321a906c41fd2f537fd3370c281e18f17f50

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]