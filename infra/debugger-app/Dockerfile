FROM gcr.io/skia-public/debugger-app-base@sha256:60e75023b30b4283704e1dfe5bcf5f6ae622c527d87e069729844061c8231d7f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]