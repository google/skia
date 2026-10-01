FROM gcr.io/skia-public/debugger-app-base@sha256:b69748450f359d48f9eb58c511bb84ba150d8ff3630314acf3159ead2ec32555

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]