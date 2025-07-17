FROM gcr.io/skia-public/debugger-app-base@sha256:f4f588b1198d5c21a844562c66cc92ef0109ead9ea3aaea1b834252288cb0f55

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js

CMD ["/usr/local/bin/debugger", "--help"]