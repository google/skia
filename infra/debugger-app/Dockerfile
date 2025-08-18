FROM gcr.io/skia-public/debugger-app-base@sha256:6943b399ac365aaeccd348d2023096a9bf4b600cab2525c07d70e1745cb55697

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]