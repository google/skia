FROM gcr.io/skia-public/debugger-app-base@sha256:17223bbc481df03e298164809d53fdb68ff4212618c227267c72c38dcd2a72c5

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js
COPY index.d.ts /usr/local/share/debugger-app/index.d.ts

CMD ["/usr/local/bin/debugger", "--help"]