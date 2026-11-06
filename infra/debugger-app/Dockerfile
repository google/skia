FROM gcr.io/skia-public/debugger-app-base@sha256:ae59ab25047d10900678ec3b24de035815c4d39436b539b5b2cdf5d7a094618f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]