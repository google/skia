FROM gcr.io/skia-public/skottie-base@sha256:e34c0f483d40fa252827b92bc4af7d9dceedf07bdc69be211218cdd0f2468e99

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]