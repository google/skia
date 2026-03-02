FROM gcr.io/skia-public/debugger-app-base@sha256:914c428338ffd2bef98f7d6b2faca3232ada189dda93b4f4b02f0a8dbfd3c75c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]