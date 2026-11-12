FROM gcr.io/skia-public/debugger-app-base@sha256:8420707762f37856e4615cf8962f71b7b9e3d19730464f44926060be61e43dc4

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]