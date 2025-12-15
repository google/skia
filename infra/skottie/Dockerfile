FROM gcr.io/skia-public/skottie-base@sha256:dfbd2869b93ae40b3781af47d8261ad5455ccc3b80ac2f21cb0f80218cc3dc44

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]