FROM gcr.io/skia-public/debugger-app-base@sha256:2ad3bc7e28434007d6e1d6e524f0fbbdaf4746b816a0ca71ede917c3cd34c482

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]