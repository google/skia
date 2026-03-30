FROM gcr.io/skia-public/debugger-app-base@sha256:60a6ebd2de6fb1df77671d8df538b39ba86ee9ba8253aed5f0a12a7b1e89359d

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]