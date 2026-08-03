FROM gcr.io/skia-public/debugger-app-base@sha256:72e9d968d26acd3fcf81a10ecf2e54a25185c74b474cf602158f8f1f0d6daed4

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]