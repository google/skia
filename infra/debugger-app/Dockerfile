FROM gcr.io/skia-public/debugger-app-base@sha256:096004527aba8bc214ecd0cfc863956a4b25eb195ae25851538f7cb37f6109c1

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]