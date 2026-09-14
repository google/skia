FROM gcr.io/skia-public/debugger-app-base@sha256:a1fe46e011c10e7ce6cabc40ae4071ca39cc9429fa3c7eeddc2f4e64475ce40a

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]