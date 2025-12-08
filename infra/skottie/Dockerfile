FROM gcr.io/skia-public/skottie-base@sha256:cba978350d703ee634f4286d21163bdc344b5a8adbbed020d91773280c902bb9

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]