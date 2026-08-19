FROM gcr.io/skia-public/debugger-app-base@sha256:229cb5edc94fcdddb54983736148843216a8b6ebef8aad2792f923af6f90ad65

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]