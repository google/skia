FROM gcr.io/skia-public/debugger-app-base@sha256:1923fa1074e13dfddc4c80b665858bfb952ddbce0aad18b10c95600182bc268f

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]