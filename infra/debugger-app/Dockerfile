FROM gcr.io/skia-public/debugger-app-base@sha256:4711d822d73011137f3ec66f9857c084da6a2506378fb56ae9fd631c1247fb7b

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]