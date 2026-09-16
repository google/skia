FROM gcr.io/skia-public/debugger-app-base@sha256:3b6994c509e4843209bc8a3735008a5d6598b6bc9464a56047c8f5c92d834656

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]