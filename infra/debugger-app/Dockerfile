FROM gcr.io/skia-public/debugger-app-base@sha256:5d3d4b22d023fcf273d91712c00d88db42782efd899116fd1537140baa0b1612

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]