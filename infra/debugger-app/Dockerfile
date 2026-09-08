FROM gcr.io/skia-public/debugger-app-base@sha256:5a940ee701ebe325ca5c39591c9103d1455958bbf5109b54a6423e3ad28dd6b9

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]