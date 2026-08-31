FROM gcr.io/skia-public/jsfiddle-base@sha256:c08203f7a3e320fea1cb1caebfe772323e3cf567ddd0a932502fa37d85c4a8e6

COPY canvaskit.js /usr/local/share/jsfiddle/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/jsfiddle/dist/canvaskit.wasm
COPY version.js /usr/local/share/jsfiddle/dist/version.js
COPY index.d.ts /usr/local/share/jsfiddle/dist/index.d.ts

CMD ["/usr/local/bin/jsfiddle", "--help"]