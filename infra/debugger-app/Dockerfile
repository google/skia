FROM gcr.io/skia-public/debugger-app-base@sha256:959c34d2cdd380679b6d10ae67ab5661ef0e2462dac20e8007a63ae106164aa6

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]