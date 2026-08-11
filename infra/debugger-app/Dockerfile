FROM gcr.io/skia-public/debugger-app-base@sha256:6f9c650f0c475a1fd13754cdee9c0ba21fb363332a6a236aeb166954f4f6fdb0

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]