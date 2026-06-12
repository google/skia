FROM gcr.io/skia-public/jsfiddle-base@sha256:ab54cf0d0a660ebcf0b5273f6d769cdebbcd24d7226eff6d24988663aa37ed7c

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]