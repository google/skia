FROM gcr.io/skia-public/debugger-app-base@sha256:33d97d296d89c6b8910fc2481a17bcff5225ba8a54d92ad9182e5742a2989090

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]