FROM gcr.io/skia-public/skottie-base@sha256:b106ae90b3afa9108a1ec70370d70753cb9425575d820b956e4f5c251b82078e

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]