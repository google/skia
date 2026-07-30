FROM gcr.io/skia-public/debugger-app-base@sha256:8f7ec5882f468dacfc25a9814d01a9c3ea386e272bc1e766358497df5cf98681

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]