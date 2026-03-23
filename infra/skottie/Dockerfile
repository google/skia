FROM gcr.io/skia-public/skottie-base@sha256:cffc4a91edf74aa52d5fd96823230ce69ec5003cafd8e39bcc440c97539b746e

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]