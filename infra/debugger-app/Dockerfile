FROM gcr.io/skia-public/debugger-app-base@sha256:6197499ed379925d57bf82e7b95cec79560579e1bd22b844406afb628b8fcff6

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]