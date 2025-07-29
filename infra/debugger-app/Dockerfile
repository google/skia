FROM gcr.io/skia-public/debugger-app-base@sha256:986ca4affc8182e6467e986dec5d5ef2a49a23ccdb46cc2e1cbe07a4f4b536ad

COPY canvaskit.js /usr/local/share/debugger-app/canvaskit.js
COPY canvaskit.wasm /usr/local/share/debugger-app/canvaskit.wasm
COPY version.js /usr/local/share/debugger-app/version.js

CMD ["/usr/local/bin/debugger", "--help"]