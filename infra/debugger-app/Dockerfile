FROM gcr.io/skia-public/debugger-app-base@sha256:c88eebd234b6f008f5e831b428b7f6b18105b638a3222bf5e7730a41bf9c3c4f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]