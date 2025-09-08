FROM gcr.io/skia-public/debugger-app-base@sha256:8a197cae1e3d88aa97eb48b715c9a3cb2dacd4c5040b86fa6e4cdf1282a88cae

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]