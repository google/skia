FROM gcr.io/skia-public/debugger-app-base@sha256:a026d3da236179321bb98eb898522f0ce4857400ceeb2b4b85982c833b0a87e7

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]