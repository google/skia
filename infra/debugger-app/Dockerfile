FROM gcr.io/skia-public/debugger-app-base@sha256:816871cb56dfc040d7282caa69a61194cb564be939d94a700f2bf0a39f6cf05c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]