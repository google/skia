FROM gcr.io/skia-public/debugger-app-base@sha256:a6838c2742c7e535e9d5f72d3caf487c3aef5018488b907aac75dedf7250aba7

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]