FROM gcr.io/skia-public/debugger-app-base@sha256:18d4e53a226aaeb699194e61d3b7aaff0c8e0dce44b33acf61508639bee7f523

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]