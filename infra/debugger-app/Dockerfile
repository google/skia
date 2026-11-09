FROM gcr.io/skia-public/debugger-app-base@sha256:25d292fbc5a664c3e36507e35d04e0417333fcce0fe2740d0adc2088a43a777d

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]