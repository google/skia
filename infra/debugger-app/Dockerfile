FROM gcr.io/skia-public/debugger-app-base@sha256:fd0291e56fb6d2556c72dafb023e50f143ba1e438de20af7ed17afb35b8d6f2e

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]