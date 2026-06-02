FROM gcr.io/skia-public/debugger-app-base@sha256:d263d5e89fa51df47386f469b1099eb59a6fbe182661be067edbc1ba5ecedf32

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]