FROM gcr.io/skia-public/debugger-app-base@sha256:e2e7fdda0991d0495ca705af2df40b61d1d81075758beb9c87e479a525f044b2

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]