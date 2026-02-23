FROM gcr.io/skia-public/debugger-app-base@sha256:451d5bbc86aeee67ef6e39102f479654889d7ab4c7d156886c2e8e3ad312739f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]