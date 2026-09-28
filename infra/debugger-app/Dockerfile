FROM gcr.io/skia-public/debugger-app-base@sha256:9b0078c1061a467454d5e8c5b7e0b90e6389b4e0e71f5e8184cb9e790f84a8fb

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]