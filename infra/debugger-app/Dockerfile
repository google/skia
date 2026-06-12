FROM gcr.io/skia-public/debugger-app-base@sha256:fd083daaa58b3c5ba95696e8123f4c56933163079d9516b0db9689a483cca08c

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]