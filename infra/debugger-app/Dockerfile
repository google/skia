FROM gcr.io/skia-public/debugger-app-base@sha256:0aa48372c92356350746a256db55036f67ae61585af1175e22fcee29bc7e479a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]