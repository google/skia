FROM gcr.io/skia-public/debugger-app-base@sha256:cb6f2c1d1d52e5c714931ca53989f43b63dbcdf3551d6b6654d839a4f87b2f2e

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]