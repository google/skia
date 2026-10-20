FROM gcr.io/skia-public/debugger-app-base@sha256:485a766127d9a5359c6c439db8f2d48ebef7ec962d8356ef3fdf70f00ee041e8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]