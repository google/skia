FROM gcr.io/skia-public/debugger-app-base@sha256:0bd074379672e542535bf8792e73e42af75b0df2f88ce292bf6459137c41dcae

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]