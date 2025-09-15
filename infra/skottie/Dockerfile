FROM gcr.io/skia-public/skottie-base@sha256:663e45a448ffba1bc571ef55ca6b77da2a0941918f4e331561f09f8e76b76bf6

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]