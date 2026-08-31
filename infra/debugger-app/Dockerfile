FROM gcr.io/skia-public/debugger-app-base@sha256:653ab96531965f0da57d67cf63537bfbd7cb437f7ac7d880480c10faec6ab1d8

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]